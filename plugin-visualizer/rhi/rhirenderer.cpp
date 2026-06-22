#include <rhi/qrhi.h>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDataStream>
#include <QtMath>
#include <cstring>
#include <limits>
#include <utility>
#include "rhirenderer.h"
#include "rhimesh.h"
#include "rhicamera.h"
#include "scene/sceneobject.h"
#include "scene/truss.h"
#include "scene/scenesurface.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/dimmercapability.h"
#include "fixture/capability/pancapability.h"
#include "fixture/capability/tiltcapability.h"
#include "fixture/capability/anglecapability.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/capability/wheelslotrotationcapability.h"
#include "fixture/capability/wheelrotationcapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixturechannel.h"

namespace photon {

namespace {

QShader loadShader(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());
    qWarning() << "RhiRenderer: failed to load shader" << path;
    return QShader();
}

constexpr quint32 kFramePayload  = 24 * sizeof(float); // mat4 + vec4 lightDir + vec4 camPos
constexpr quint32 kObjectPayload = 20 * sizeof(float); // mat4 + vec4
constexpr quint32 kBeamPayload   = 36 * sizeof(float); // mat4 + color + apex + axisCos + params + color2
constexpr quint32 kGizmoBytes    = 4096 * 6 * sizeof(float); // dynamic line verts

// Beam appearance.
constexpr float kBeamLength    = 6.0f;    // reference throw (world units) at the reference angle
constexpr float kBeamHalfAngle = 8.0f;    // default/reference cone half-angle, degrees
constexpr float kBeamGain      = 0.55f;   // overall additive strength
constexpr float kBeamMinLevel  = 0.02f;   // skip beams dimmer than this
constexpr float kBeamThrowMin  = 0.5f;    // throw scale clamp (wide washes)
constexpr float kBeamThrowMax  = 2.0f;    // throw scale clamp (tight spots)

constexpr int    kGoboSize     = 1024;    // gobo texture layer resolution

// Surface lighting: spotlight list. 5 vec4 per light + 1 vec4 count header.
constexpr int    kMaxLights    = 32;
constexpr quint32 kLightsPayload = (1 + 5 * kMaxLights) * 4 * sizeof(float);
constexpr float  kGoboRevPerSec = 0.5f;   // full-speed gobo rotation rate (rev/s)
constexpr float  kColorSlotsPerSec = 1.5f; // full-speed color-wheel scroll (slots/s)
constexpr float  kColorGateWidth = 0.5f;  // gate window in slot units (split when crossing a boundary)

// Pan/tilt motor model (degrees). Real heads ease in/out and cruise at a capped
// speed; SmoothDamp reproduces that with a responsiveness time + a max-speed clamp.
constexpr float  kAimSmoothTime = 0.30f;  // responsiveness (s)
constexpr float  kPanMaxSpeed   = 300.0f; // deg/s
constexpr float  kTiltMaxSpeed  = 200.0f; // deg/s
constexpr float  kZoomSmoothTime = 0.40f; // zoom (beam half-angle) easing
constexpr float  kZoomMaxSpeed  = 45.0f;  // deg/s of half-angle
constexpr float  kGoboSlotsPerSec = 4.0f; // gobo wheel slew (layers/s)

// Moves a wheel position toward a target along the shortest path on a circle of
// `count` slots, at most `maxStep` this frame. Result wrapped into [0, count).
float slewWrapped(float pos, float target, float count, float maxStep)
{
    float d = target - pos;
    d -= count * std::round(d / count);          // shortest signed distance
    pos += (std::abs(d) <= maxStep) ? d : (d > 0.0f ? maxStep : -maxStep);
    pos = std::fmod(pos, count);
    if (pos < 0.0f) pos += count;
    return pos;
}

// Critically-damped smoothing toward a target with a max-speed clamp (Unity-style
// SmoothDamp). Eases out of rest, cruises at maxSpeed for long moves, eases in.
float smoothDamp(float current, float target, float &vel,
                 float smoothTime, float maxSpeed, float dt)
{
    if (dt <= 0.0f)
        return current;
    smoothTime = qMax(0.0001f, smoothTime);
    const float omega = 2.0f / smoothTime;
    const float x = omega * dt;
    const float expf = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    const float maxChange = maxSpeed * smoothTime;
    change = qBound(-maxChange, change, maxChange);
    const float t = current - change;             // clamped target
    const float temp = (vel + omega * change) * dt;
    vel = (vel - omega * temp) * expf;
    float out = t + (change + temp) * expf;
    // Prevent overshoot past the original target.
    if ((target - current > 0.0f) == (out > target)) {
        out = target;
        vel = (out - target) / dt;
    }
    return out;
}

QColor multiplyColor(const QColor &a, const QColor &b)
{
    return QColor::fromRgbF(float(a.redF() * b.redF()),
                            float(a.greenF() * b.greenF()),
                            float(a.blueF() * b.blueF()));
}

// Derives the world-space cone (apex, axis, cos half-angle, length) from a beam's
// model matrix (unit cone: apex at origin, axis -Y, base ring radius 1 at y = -1).
void beamConeFromModel(const QMatrix4x4 &model, QVector3D &apex, QVector3D &axis,
                       float &cosH, float &length)
{
    apex = model.map(QVector3D(0.0f, 0.0f, 0.0f));
    const QVector3D baseC = model.map(QVector3D(0.0f, -1.0f, 0.0f));
    const QVector3D rim   = model.map(QVector3D(1.0f, -1.0f, 0.0f));
    axis = baseC - apex;
    length = axis.length();
    if (length > 1e-5f)
        axis /= length;
    const float radius = (rim - baseC).length();
    const float tanH = (length > 1e-5f) ? radius / length : 0.1f;
    cosH = 1.0f / std::sqrt(1.0f + tanH * tanH);
}

} // namespace

RhiRenderer::RhiRenderer() {}

RhiRenderer::~RhiRenderer()
{
    releaseResources();
    delete m_box;
    delete m_grid;
    delete m_beamCone;
    delete m_plane;
}

void RhiRenderer::releaseResources()
{
    delete m_meshPipeline;    m_meshPipeline = nullptr;
    delete m_surfacePipeline; m_surfacePipeline = nullptr;
    delete m_linePipeline;    m_linePipeline = nullptr;
    delete m_beamPipeline;    m_beamPipeline = nullptr;
    delete m_beamVolPipeline; m_beamVolPipeline = nullptr;
    delete m_gizmoPipeline;   m_gizmoPipeline = nullptr;
    delete m_meshSrb;         m_meshSrb = nullptr;
    delete m_lineSrb;         m_lineSrb = nullptr;
    delete m_beamSrb;         m_beamSrb = nullptr;
    delete m_surfaceSrb;      m_surfaceSrb = nullptr;
    delete m_frameBuffer;     m_frameBuffer = nullptr;
    delete m_objectBuffer;    m_objectBuffer = nullptr;
    delete m_beamBuffer;      m_beamBuffer = nullptr;
    delete m_lightsBuffer;    m_lightsBuffer = nullptr;
    delete m_gizmoBuffer;     m_gizmoBuffer = nullptr;
    delete m_goboTex;         m_goboTex = nullptr;
    delete m_goboSampler;     m_goboSampler = nullptr;
    m_goboUploaded = false;
    m_objectCapacity = 0;
    m_beamCapacity = 0;

    if (m_box)      m_box->release();
    if (m_grid)     m_grid->release();
    if (m_beamCone) m_beamCone->release();
    if (m_plane)    m_plane->release();

    qDeleteAll(m_trussMeshes);
    m_trussMeshes.clear();
    m_trussSig.clear();

    m_rhi = nullptr;
}

void RhiRenderer::ensureObjectBuffer(int count)
{
    if (count <= m_objectCapacity && m_objectBuffer)
        return;

    const int newCap = qMax(count, 64);

    delete m_objectBuffer;
    m_objectBuffer = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer,
                                      m_objectSlotSize * newCap);
    m_objectBuffer->create();
    m_objectCapacity = newCap;

    // The mesh SRB references m_objectBuffer by pointer, so rebuild it.
    if (m_frameBuffer) {
        delete m_meshSrb;
        m_meshSrb = m_rhi->newShaderResourceBindings();
        const auto stages = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
        m_meshSrb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, stages, m_frameBuffer),
            QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(1, stages, m_objectBuffer, kObjectPayload)
        });
        m_meshSrb->create();
    }

    // The surface SRB also references m_objectBuffer (binding 1), so rebuild it too.
    if (m_frameBuffer && m_lightsBuffer && m_goboTex && m_goboSampler) {
        const auto stages = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
        delete m_surfaceSrb;
        m_surfaceSrb = m_rhi->newShaderResourceBindings();
        m_surfaceSrb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, stages, m_frameBuffer),
            QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(1, stages, m_objectBuffer, kObjectPayload),
            QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_lightsBuffer),
            QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage, m_goboTex, m_goboSampler)
        });
        m_surfaceSrb->create();
    }
}

void RhiRenderer::ensureBeamBuffer(int count)
{
    if (count <= m_beamCapacity && m_beamBuffer)
        return;

    const int newCap = qMax(count, 32);

    delete m_beamBuffer;
    m_beamBuffer = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer,
                                    m_beamSlotSize * newCap);
    m_beamBuffer->create();
    m_beamCapacity = newCap;

    // The beam SRB references m_beamBuffer by pointer, so rebuild it.
    if (m_frameBuffer && m_goboTex && m_goboSampler) {
        delete m_beamSrb;
        m_beamSrb = m_rhi->newShaderResourceBindings();
        const auto stages = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
        m_beamSrb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, stages, m_frameBuffer),
            QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(1, stages, m_beamBuffer, kBeamPayload),
            QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, m_goboTex, m_goboSampler)
        });
        m_beamSrb->create();
    }
}

void RhiRenderer::initialize(QRhi *rhi, QRhiRenderPassDescriptor *rpDesc, int sampleCount)
{
    m_rhi = rhi;

    if (!m_clock.isValid())
        m_clock.start();

    if (!m_box)
        m_box = RhiMesh::createBox(0.15f, 0.15f, 0.15f);
    if (!m_grid)
        m_grid = RhiMesh::createGrid(20, 1.0f);
    if (!m_beamCone)
        m_beamCone = RhiMesh::createCone(28);
    if (!m_plane)
        m_plane = RhiMesh::createPlane();

    // Per-object slot strides must satisfy the uniform-buffer offset alignment.
    m_objectSlotSize = m_rhi->ubufAlignment();
    while (m_objectSlotSize < kObjectPayload)
        m_objectSlotSize += m_rhi->ubufAlignment();

    m_beamSlotSize = m_rhi->ubufAlignment();
    while (m_beamSlotSize < kBeamPayload)
        m_beamSlotSize += m_rhi->ubufAlignment();

    m_frameBuffer = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, kFramePayload);
    m_frameBuffer->create();

    // Spotlight list for surface lighting. Created before the object buffer so the
    // surface SRB (which references it) can be built inside ensureObjectBuffer().
    m_lightsBuffer = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, kLightsPayload);
    m_lightsBuffer->create();

    // Gobo texture array + sampler. Created before the SRBs (which sample it). At
    // least one layer always exists so the bindings are valid even with no images.
    loadGoboImages();
    const int goboLayers = qMax(1, m_goboImages.size());
    m_goboTex = m_rhi->newTextureArray(QRhiTexture::RGBA8, goboLayers, QSize(kGoboSize, kGoboSize), 1,
                                       QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);
    m_goboTex->create();
    m_goboSampler = m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::Linear,
                                      QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
    m_goboSampler->create();
    m_goboUploaded = false;

    // Creates m_objectBuffer, m_meshSrb and m_surfaceSrb.
    ensureObjectBuffer(64);
    // Creates m_beamBuffer and m_beamSrb.
    ensureBeamBuffer(32);

    m_lineSrb = m_rhi->newShaderResourceBindings();
    m_lineSrb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_frameBuffer)
    });
    m_lineSrb->create();

    const QShader sceneVert = loadShader(QStringLiteral(":/visualizer/shaders/scene.vert.qsb"));
    const QShader sceneFrag = loadShader(QStringLiteral(":/visualizer/shaders/scene.frag.qsb"));
    const QShader lineVert  = loadShader(QStringLiteral(":/visualizer/shaders/line.vert.qsb"));
    const QShader lineFrag  = loadShader(QStringLiteral(":/visualizer/shaders/line.frag.qsb"));
    const QShader beamVert  = loadShader(QStringLiteral(":/visualizer/shaders/beam.vert.qsb"));
    const QShader beamFrag  = loadShader(QStringLiteral(":/visualizer/shaders/beam.frag.qsb"));
    const QShader beamVolVert = loadShader(QStringLiteral(":/visualizer/shaders/beamvol.vert.qsb"));
    const QShader beamVolFrag = loadShader(QStringLiteral(":/visualizer/shaders/beamvol.frag.qsb"));
    const QShader surfaceVert = loadShader(QStringLiteral(":/visualizer/shaders/surface.vert.qsb"));
    const QShader surfaceFrag = loadShader(QStringLiteral(":/visualizer/shaders/surface.frag.qsb"));

    // Both vertex layouts are vec3 + vec3, 24-byte stride.
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ { 6 * sizeof(float) } });
    inputLayout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }
    });

    // Lit mesh pipeline.
    m_meshPipeline = m_rhi->newGraphicsPipeline();
    m_meshPipeline->setShaderStages({
        { QRhiShaderStage::Vertex, sceneVert },
        { QRhiShaderStage::Fragment, sceneFrag }
    });
    m_meshPipeline->setVertexInputLayout(inputLayout);
    m_meshPipeline->setShaderResourceBindings(m_meshSrb);
    m_meshPipeline->setRenderPassDescriptor(rpDesc);
    m_meshPipeline->setSampleCount(sampleCount);
    m_meshPipeline->setDepthTest(true);
    m_meshPipeline->setDepthWrite(true);
    m_meshPipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_meshPipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    m_meshPipeline->create();

    // Surface pipeline: spotlight-lit planes (walls / floor). Opaque, double-sided.
    m_surfacePipeline = m_rhi->newGraphicsPipeline();
    m_surfacePipeline->setShaderStages({
        { QRhiShaderStage::Vertex, surfaceVert },
        { QRhiShaderStage::Fragment, surfaceFrag }
    });
    m_surfacePipeline->setVertexInputLayout(inputLayout);
    m_surfacePipeline->setShaderResourceBindings(m_surfaceSrb);
    m_surfacePipeline->setRenderPassDescriptor(rpDesc);
    m_surfacePipeline->setSampleCount(sampleCount);
    m_surfacePipeline->setDepthTest(true);
    m_surfacePipeline->setDepthWrite(true);
    m_surfacePipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_surfacePipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    m_surfacePipeline->create();

    // Line pipeline for the grid.
    m_linePipeline = m_rhi->newGraphicsPipeline();
    m_linePipeline->setShaderStages({
        { QRhiShaderStage::Vertex, lineVert },
        { QRhiShaderStage::Fragment, lineFrag }
    });
    m_linePipeline->setVertexInputLayout(inputLayout);
    m_linePipeline->setShaderResourceBindings(m_lineSrb);
    m_linePipeline->setRenderPassDescriptor(rpDesc);
    m_linePipeline->setSampleCount(sampleCount);
    m_linePipeline->setDepthTest(true);
    m_linePipeline->setDepthWrite(true);
    m_linePipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_linePipeline->setTopology(QRhiGraphicsPipeline::Lines);
    m_linePipeline->create();

    // Beam pipeline: additive translucent cones. Same vertex layout and per-object
    // SRB as the mesh pass, but additive blending with depth-write disabled so beams
    // glow over the scene and over each other without occluding.
    QRhiGraphicsPipeline::TargetBlend beamBlend;
    beamBlend.enable = true;
    beamBlend.srcColor = QRhiGraphicsPipeline::One;
    beamBlend.dstColor = QRhiGraphicsPipeline::One;
    beamBlend.srcAlpha = QRhiGraphicsPipeline::One;
    beamBlend.dstAlpha = QRhiGraphicsPipeline::One;

    m_beamPipeline = m_rhi->newGraphicsPipeline();
    m_beamPipeline->setShaderStages({
        { QRhiShaderStage::Vertex, beamVert },
        { QRhiShaderStage::Fragment, beamFrag }
    });
    m_beamPipeline->setVertexInputLayout(inputLayout);
    m_beamPipeline->setShaderResourceBindings(m_meshSrb);
    m_beamPipeline->setRenderPassDescriptor(rpDesc);
    m_beamPipeline->setSampleCount(sampleCount);
    m_beamPipeline->setTargetBlends({ beamBlend });
    m_beamPipeline->setDepthTest(true);
    m_beamPipeline->setDepthWrite(false);
    m_beamPipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_beamPipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    m_beamPipeline->create();

    // Volumetric beam pipeline: raymarch in the fragment shader. Same additive
    // blend/depth setup as the basic cone but uses the per-beam param SRB. Cull
    // none so any interior pixel is covered (the integral is view-ray analytic, so
    // extra covering fragments only scale brightness, absorbed into the gain).
    m_beamVolPipeline = m_rhi->newGraphicsPipeline();
    m_beamVolPipeline->setShaderStages({
        { QRhiShaderStage::Vertex, beamVolVert },
        { QRhiShaderStage::Fragment, beamVolFrag }
    });
    m_beamVolPipeline->setVertexInputLayout(inputLayout);
    m_beamVolPipeline->setShaderResourceBindings(m_beamSrb);
    m_beamVolPipeline->setRenderPassDescriptor(rpDesc);
    m_beamVolPipeline->setSampleCount(sampleCount);
    m_beamVolPipeline->setTargetBlends({ beamBlend });
    m_beamVolPipeline->setDepthTest(true);
    m_beamVolPipeline->setDepthWrite(false);
    m_beamVolPipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_beamVolPipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    m_beamVolPipeline->create();

    // Gizmo: same line shaders/layout/SRB, but depth test off so it draws on top.
    m_gizmoBuffer = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, kGizmoBytes);
    m_gizmoBuffer->create();

    m_gizmoPipeline = m_rhi->newGraphicsPipeline();
    m_gizmoPipeline->setShaderStages({
        { QRhiShaderStage::Vertex, lineVert },
        { QRhiShaderStage::Fragment, lineFrag }
    });
    m_gizmoPipeline->setVertexInputLayout(inputLayout);
    m_gizmoPipeline->setShaderResourceBindings(m_lineSrb);
    m_gizmoPipeline->setRenderPassDescriptor(rpDesc);
    m_gizmoPipeline->setSampleCount(sampleCount);
    m_gizmoPipeline->setDepthTest(false);
    m_gizmoPipeline->setDepthWrite(false);
    m_gizmoPipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_gizmoPipeline->setTopology(QRhiGraphicsPipeline::Lines);
    m_gizmoPipeline->create();
}

RhiMesh *RhiRenderer::trussMeshFor(SceneObject *obj)
{
    auto *truss = static_cast<Truss *>(obj);

    // Signature of the parameters that affect geometry; rebuild when it changes.
    QByteArray sig;
    {
        QDataStream s(&sig, QIODevice::WriteOnly);
        s << truss->beams() << truss->length() << truss->offset()
          << truss->radius() << truss->segmentLength();
    }

    auto meshIt = m_trussMeshes.find(obj);
    if (meshIt != m_trussMeshes.end() && m_trussSig.value(obj) == sig)
        return meshIt.value();

    // (Re)build.
    if (meshIt != m_trussMeshes.end()) {
        meshIt.value()->release();
        delete meshIt.value();
    }

    RhiMesh *mesh = RhiMesh::createTruss(int(truss->beams()), truss->length(),
                                         truss->offset(), truss->radius(),
                                         truss->segmentLength());
    m_trussMeshes.insert(obj, mesh);
    m_trussSig.insert(obj, sig);
    return mesh;
}

void RhiRenderer::collectDrawables(SceneObject *obj, QVector<Drawable> &out,
                                   QSet<SceneObject *> &seenTrusses)
{
    if (!obj)
        return;

    const QColor highlight(255, 170, 40);

    for (SceneObject *child : obj->sceneChildren()) {
        const QByteArray type = child->typeId();
        const bool sel = (child == m_selected);
        if (type == "fixture") {
            QColor tint(150, 150, 158);  // body color for fixtures we can't evaluate
            if (sel) {
                tint = highlight;
            } else {
                QColor emitted;
                float intensity = 0.0f;
                if (evaluateFixture(static_cast<Fixture *>(child), emitted, intensity)) {
                    // Body = dark "off" gray faded out by intensity, plus the emitted
                    // color added on top, so an unlit fixture reads dark and a lit one
                    // glows in its real color.
                    const float keep = 1.0f - intensity;
                    tint = QColor::fromRgbF(
                        qMin(1.0f, 0.28f * keep + float(emitted.redF())),
                        qMin(1.0f, 0.28f * keep + float(emitted.greenF())),
                        qMin(1.0f, 0.30f * keep + float(emitted.blueF())));
                }
            }
            out.append({ m_box, child->globalMatrix(), tint });
        } else if (type == "truss") {
            seenTrusses.insert(child);
            out.append({ trussMeshFor(child), child->globalMatrix(), sel ? highlight : QColor(150, 150, 155) });
        } else if (type != "group") {
            out.append({ m_box, child->globalMatrix(), sel ? highlight : QColor(150, 130, 95) });
        }
        // "group" is organizational — no geometry, but still recurse into it.
        collectDrawables(child, out, seenTrusses);
    }
}

bool RhiRenderer::evaluateFixture(Fixture *fixture, QColor &outColor, float &outIntensity) const
{
    if (!fixture)
        return false;

    // ColorCapability is an aggregate (not stored per-channel), so it must be looked
    // up by type rather than via the channel-walking findCapability<T>() template.
    const auto colorCaps  = fixture->findCapability(Capability_Color);
    const auto dimmerCaps = fixture->findCapability(Capability_Dimmer);
    if (colorCaps.isEmpty() && dimmerCaps.isEmpty())
        return false;

    // Colorless fixtures (e.g. a plain dimmer) emit white; the dimmer (if any)
    // scales it. For color fixtures with no separate dimmer the level is already
    // carried by the color channels, so dim stays 1.0.
    const QColor base = colorCaps.isEmpty()
        ? QColor(255, 255, 255)
        : static_cast<ColorCapability *>(colorCaps.first())->getColor(m_dmx);
    const float dim = dimmerCaps.isEmpty()
        ? 1.0f
        : float(static_cast<DimmerCapability *>(dimmerCaps.first())->getPercent(m_dmx));

    const float r = qBound(0.0f, float(base.redF())   * dim, 1.0f);
    const float g = qBound(0.0f, float(base.greenF()) * dim, 1.0f);
    const float b = qBound(0.0f, float(base.blueF())  * dim, 1.0f);

    outColor = QColor::fromRgbF(r, g, b);
    outIntensity = qMax(r, qMax(g, b));
    return true;
}

float RhiRenderer::beamHalfAngleFor(Fixture *fixture) const
{
    const auto zooms = fixture->findCapability(Capability_Zoom);
    if (zooms.isEmpty())
        return kBeamHalfAngle;

    // Zoom percent maps across the fixture's physical lens range to a full beam angle.
    auto *zoom = static_cast<AngleCapability *>(zooms.first());
    const Fixture::Physical phys = fixture->physical();
    const double pct = zoom->getAnglePercent(m_dmx);
    const double fullAngle = pct * (phys.lensMaximum - phys.lensMinimum) + phys.lensMinimum;
    return qBound(1.0f, float(fullAngle) * 0.5f, 80.0f);
}

void RhiRenderer::loadGoboImages()
{
    if (!m_goboImages.isEmpty())
        return;

    const QString dir = QCoreApplication::applicationDirPath() + "/fixtures/gobos";
    QDir gd(dir);
    const QStringList files = gd.entryList(QStringList() << "*.png", QDir::Files, QDir::Name);
    for (const QString &f : files) {
        QImage img(gd.filePath(f));
        if (img.isNull())
            continue;
        img = img.convertToFormat(QImage::Format_RGBA8888);
        if (img.size() != QSize(kGoboSize, kGoboSize))
            img = img.scaled(kGoboSize, kGoboSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_goboImages.append(img);
    }
    qInfo() << "RhiRenderer: loaded" << m_goboImages.size() << "gobos from" << dir;
}

float RhiRenderer::goboForFixture(Fixture *fixture) const
{
    const int gc = m_goboImages.size();
    if (gc <= 0)
        return 0.0f;   // no gobo textures loaded

    const auto slotCaps = fixture->findCapability<WheelSlotCapability *>();
    if (slotCaps.isEmpty())
        return float(qBound(0, m_goboIndex, gc));   // no wheel — follow global selector

    // The fixture has a gobo wheel (it owns its gobo). Default to open (no gobo);
    // the active gobo slot maps onto a loaded gobo texture layer.
    bool hasGoboWheel = false;
    int active = 0;
    for (auto *cap : slotCaps) {
        FixtureWheelSlot *ws = cap->wheelSlot();
        if (!ws)
            continue;
        if (ws->type() == FixtureWheelSlot::Slot_Gobo)
            hasGoboWheel = true;
        if (cap->isValid(m_dmx))
            active = (ws->type() == FixtureWheelSlot::Slot_Gobo) ? (ws->index() % gc) + 1 : 0;
    }
    if (!hasGoboWheel)
        return float(qBound(0, m_goboIndex, gc));   // global test selector snaps

    // Slew the wheel toward the selected gobo so it steps through the intervening
    // slots (a quick sweep) rather than snapping. Snap on first sight.
    const float target = float(active);
    if (!m_goboWheelPos.contains(fixture)) {
        m_goboWheelPos[fixture] = target;
        return target;
    }
    float &gp = m_goboWheelPos[fixture];
    const float maxStep = kGoboSlotsPerSec * m_frameDt;
    const float d = target - gp;
    gp += (std::abs(d) <= maxStep) ? d : (d > 0.0f ? maxStep : -maxStep);
    return float(std::round(gp));
}

float RhiRenderer::goboRotationFor(Fixture *fixture) const
{
    const auto rots = fixture->findCapability<WheelSlotRotationCapability *>();
    if (rots.isEmpty())
        return 0.0f;

    for (auto *cap : rots) {
        if (!cap->isValid(m_dmx) || !cap->channel())
            continue;

        // Channel value as a 0..1 fraction within this capability's DMX range.
        const int raw = m_dmx.value(fixture->universe() - 1, cap->channel()->universalChannelNumber());
        DMXRange r = cap->range();
        const float span = float(qMax(1, int(r.end) - int(r.start)));
        const float factor = qBound(0.0f, (float(raw) - float(r.start)) / span, 1.0f);

        if (cap->supportsAngle()) {
            // Indexed gobo: static angle within the range.
            const double deg = cap->angleStart() + factor * (cap->angleEnd() - cap->angleStart());
            m_goboPhase[fixture] = float(qDegreesToRadians(deg));
            return m_goboPhase[fixture];
        }

        // Continuous rotation: normalized speed (-1..1) accumulated over time.
        const double speedNorm = cap->speedStart() + factor * (cap->speedEnd() - cap->speedStart());
        float &phase = m_goboPhase[fixture];
        phase += float(speedNorm) * kGoboRevPerSec * 2.0f * float(M_PI) * m_frameDt;
        return phase;
    }

    // Active capability not in a rotation range (e.g. stop) — hold current angle.
    return m_goboPhase.value(fixture, 0.0f);
}

bool RhiRenderer::colorWheelFor(Fixture *fixture, QColor &outA, QColor &outB, float &outSplit) const
{
    // Locate the colour wheel via any colour slot capability.
    const auto slotCaps = fixture->findCapability<WheelSlotCapability *>();
    QString wheelName;
    for (auto *cap : slotCaps) {
        FixtureWheelSlot *ws = cap->wheelSlot();
        if (ws && ws->type() == FixtureWheelSlot::Slot_Color) {
            wheelName = cap->wheelName();
            break;
        }
    }
    if (wheelName.isEmpty())
        return false;

    FixtureWheel *wheel = fixture->findWheel(wheelName);
    if (!wheel)
        return false;

    // Ordered colour strip (open / non-colour slots read as white).
    QVector<QColor> strip;
    for (FixtureWheelSlot *s : wheel->allSlots()) {
        if (s && s->type() == FixtureWheelSlot::Slot_Color)
            strip.append(static_cast<ColorSlot *>(s)->color());
        else
            strip.append(QColor(255, 255, 255));
    }
    const int N = strip.size();
    if (N == 0)
        return false;

    // Continuous wheel position in slot units.
    float pos = 0.0f;
    bool active = false;

    for (auto *cap : slotCaps) {     // a discrete slot is selected
        FixtureWheelSlot *ws = cap->wheelSlot();
        if (ws && ws->type() == FixtureWheelSlot::Slot_Color && cap->isValid(m_dmx)) {
            const int slotIdx = ((cap->slotNumber() - 1) % N + N) % N;
            // Slew the wheel toward the selected slot (motor): the gate then sweeps
            // through the intervening colours rather than snapping.
            float &cp = m_colorPhase[fixture];
            cp = slewWrapped(cp, float(slotIdx), float(N), kColorSlotsPerSec * m_frameDt);
            pos = cp;
            active = true;

            // Once parked on the slot, honour an authored two-colour (split) slot.
            const QVector<QColor> cols = static_cast<ColorSlot *>(ws)->colors();
            if (cols.size() >= 2 && std::abs(pos - float(slotIdx)) < 0.02f) {
                outA = cols[0];
                outB = cols[1];
                outSplit = 0.0f;
                return true;
            }
            break;
        }
    }

    if (!active) {                   // the wheel is spinning through colours
        const auto rots = fixture->findCapability<WheelRotationCapability *>();
        for (auto *cap : rots) {
            if (cap->wheelName().toLower() != wheelName.toLower() || !cap->isValid(m_dmx) || !cap->channel())
                continue;
            const int raw = m_dmx.value(fixture->universe() - 1, cap->channel()->universalChannelNumber());
            DMXRange r = cap->range();
            const float span = float(qMax(1, int(r.end) - int(r.start)));
            const float factor = qBound(0.0f, (float(raw) - float(r.start)) / span, 1.0f);
            const double speedNorm = cap->speedStart() + factor * (cap->speedEnd() - cap->speedStart());
            float &phase = m_colorPhase[fixture];
            phase += float(speedNorm) * kColorSlotsPerSec * m_frameDt;
            phase = std::fmod(phase, float(N));
            if (phase < 0.0f) phase += float(N);
            pos = phase;
            active = true;
            break;
        }
    }

    if (!active)
        return false;                // open / no colour filter in the gate

    // Resolve the gate (a window of width kColorGateWidth slots) to one or two
    // colours plus a split position across the beam.
    const float w = kColorGateWidth;
    const float lo = pos - w * 0.5f;
    const float hi = pos + w * 0.5f;
    auto slotAt = [&](float p) { int k = int(std::floor(p + 0.5f)); return ((k % N) + N) % N; };
    const int kA = slotAt(lo);
    const int kB = slotAt(hi);
    outA = strip[kA];
    outB = strip[kB];
    if (kA == kB) {
        outSplit = -2.0f;            // solid colour, no split line
    } else {
        float b1 = std::floor(lo) + 0.5f;            // the boundary half-integer in (lo,hi)
        const float bx = (b1 > lo && b1 < hi) ? b1 : std::floor(hi) + 0.5f;
        const float frac = (bx - lo) / w;            // 0..1 across the gate
        outSplit = qBound(-1.0f, frac * 2.0f - 1.0f, 1.0f);
    }
    return true;
}

void RhiRenderer::updateFixtureMotion(Fixture *fixture, float &panOut, float &tiltOut,
                                      float &halfAngleOut) const
{
    // DMX target angles, centered so 50% = straight down the fixture's local -Y.
    float panTarget = 0.0f, tiltTarget = 0.0f;
    const auto pans = fixture->findCapability(Capability_Pan);
    if (!pans.isEmpty()) {
        auto *p = static_cast<PanCapability *>(pans.first());
        const float range = float(p->angleEnd() - p->angleStart());
        panTarget = float(p->getAnglePercent(m_dmx)) * range - range * 0.5f;
    }
    const auto tilts = fixture->findCapability(Capability_Tilt);
    if (!tilts.isEmpty()) {
        auto *t = static_cast<TiltCapability *>(tilts.first());
        const float range = float(t->angleEnd() - t->angleStart());
        tiltTarget = float(t->getAnglePercent(m_dmx)) * range - range * 0.5f;
    }
    const float zoomTarget = beamHalfAngleFor(fixture);

    FixtureMotion &mo = m_motion[fixture];
    if (!mo.init) {
        // Snap on first sight (don't animate from rest on load).
        mo.pan = panTarget;
        mo.tilt = tiltTarget;
        mo.zoom = zoomTarget;
        mo.init = true;
    } else {
        // Linear slew (no shortest-angle wrap — real heads have end-stops).
        mo.pan  = smoothDamp(mo.pan,  panTarget,  mo.panVel,  kAimSmoothTime, kPanMaxSpeed,  m_frameDt);
        mo.tilt = smoothDamp(mo.tilt, tiltTarget, mo.tiltVel, kAimSmoothTime, kTiltMaxSpeed, m_frameDt);
        mo.zoom = smoothDamp(mo.zoom, zoomTarget, mo.zoomVel, kZoomSmoothTime, kZoomMaxSpeed, m_frameDt);
    }
    panOut = mo.pan;
    tiltOut = mo.tilt;
    halfAngleOut = mo.zoom;
}

void RhiRenderer::collectBeams(SceneObject *obj, QVector<Drawable> &out) const
{
    if (!obj || !m_beamCone)
        return;

    const float refTan = std::tan(qDegreesToRadians(kBeamHalfAngle));

    for (SceneObject *child : obj->sceneChildren()) {
        if (child->typeId() == "fixture") {
            auto *fix = static_cast<Fixture *>(child);

            // Advance pan/tilt/zoom motors every frame (even when dark, so the head
            // tracks during blackout "dark moves").
            float panDeg = 0.0f, tiltDeg = 0.0f, halfAngle = 8.0f;
            updateFixtureMotion(fix, panDeg, tiltDeg, halfAngle);

            QColor emitted;
            float intensity = 0.0f;
            if (evaluateFixture(fix, emitted, intensity) && intensity > kBeamMinLevel) {
                // Beam width follows zoom; tighter beams throw farther (clamped).
                const float tanHalf = std::tan(qDegreesToRadians(halfAngle));
                const float length = kBeamLength
                    * qBound(kBeamThrowMin, refTan / tanHalf, kBeamThrowMax);
                const float baseRadius = length * tanHalf;

                // Unit cone (apex at origin, axis -Y) oriented and sized into place.
                QMatrix4x4 m = fix->globalMatrix();
                m.rotate(panDeg, 0.0f, 1.0f, 0.0f);
                m.rotate(tiltDeg, 1.0f, 0.0f, 0.0f);
                m.scale(baseRadius, length, baseRadius);

                // Apply the colour wheel (tints the emitted colour; may be split).
                QColor colA = emitted, colB = emitted;
                float split = -2.0f;
                QColor wa, wb;
                float ws = -2.0f;
                if (colorWheelFor(fix, wa, wb, ws)) {
                    colA = multiplyColor(emitted, wa);
                    colB = multiplyColor(emitted, wb);
                    split = ws;
                }

                Drawable beam;
                beam.mesh = m_beamCone;
                beam.model = m;
                beam.color = QColor::fromRgbF(float(colA.redF()), float(colA.greenF()),
                                              float(colA.blueF()), kBeamGain);
                beam.gobo = goboForFixture(fix);
                beam.goboRot = goboRotationFor(fix);
                beam.color2 = QColor::fromRgbF(float(colB.redF()), float(colB.greenF()),
                                               float(colB.blueF()), kBeamGain);
                beam.split = split;
                out.append(beam);
            }
        }
        collectBeams(child, out);
    }
}

void RhiRenderer::collectSurfaces(SceneObject *obj, QVector<Drawable> &out) const
{
    if (!obj || !m_plane)
        return;

    for (SceneObject *child : obj->sceneChildren()) {
        if (child->typeId() == "surface") {
            auto *surf = static_cast<SceneSurface *>(child);
            QMatrix4x4 m = child->globalMatrix();
            m.scale(surf->surfaceWidth(), surf->surfaceHeight(), 1.0f);
            out.append({ m_plane, m, surf->color() });
        }
        collectSurfaces(child, out);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Picking
// ─────────────────────────────────────────────────────────────────────────────

void RhiRenderer::setSelection(SceneObject *obj)
{
    m_selected = obj;
    m_gizmo.setTarget(obj);
}

bool RhiRenderer::localBounds(SceneObject *obj, QVector3D &outMin, QVector3D &outMax)
{
    const QByteArray type = obj->typeId();
    if (type == "truss") {
        auto *tr = static_cast<Truss *>(obj);
        const float hx = tr->length() * 0.5f;
        const float r = tr->offset() + tr->radius();
        outMin = QVector3D(-hx, -r, -r);
        outMax = QVector3D( hx,  r,  r);
        return true;
    }
    if (type == "surface") {
        auto *surf = static_cast<SceneSurface *>(obj);
        const float hw = surf->surfaceWidth() * 0.5f;
        const float hh = surf->surfaceHeight() * 0.5f;
        outMin = QVector3D(-hw, -hh, -0.05f);
        outMax = QVector3D( hw,  hh,  0.05f);
        return true;
    }
    if (type == "fixture" || type == "arrow" || type != "group") {
        // Same half-extents as the box mesh.
        outMin = QVector3D(-0.15f, -0.15f, -0.15f);
        outMax = QVector3D( 0.15f,  0.15f,  0.15f);
        return true;
    }
    return false; // groups are not selectable
}

namespace {

// Ray vs. axis-aligned box (slab test). t is the world-ray parameter because the
// ray is transformed into the box's local space by the same affine matrix.
bool rayAabb(const QVector3D &o, const QVector3D &d,
             const QVector3D &mn, const QVector3D &mx, float &tHit)
{
    float tmin = -std::numeric_limits<float>::max();
    float tmax =  std::numeric_limits<float>::max();

    for (int a = 0; a < 3; ++a) {
        const float oa = o[a], da = d[a], mna = mn[a], mxa = mx[a];
        if (std::abs(da) < 1e-8f) {
            if (oa < mna || oa > mxa)
                return false;
        } else {
            float t1 = (mna - oa) / da;
            float t2 = (mxa - oa) / da;
            if (t1 > t2) std::swap(t1, t2);
            tmin = qMax(tmin, t1);
            tmax = qMin(tmax, t2);
            if (tmin > tmax)
                return false;
        }
    }
    if (tmax < 0.0f)
        return false; // box entirely behind the ray
    tHit = tmin > 0.0f ? tmin : tmax;
    return true;
}

} // namespace

void RhiRenderer::pickRecursive(SceneObject *obj, const QVector3D &origin, const QVector3D &dir,
                                SceneObject *&best, float &bestT) const
{
    if (!obj)
        return;

    for (SceneObject *child : obj->sceneChildren()) {
        QVector3D mn, mx;
        if (localBounds(child, mn, mx)) {
            const QMatrix4x4 inv = child->globalMatrix().inverted();
            const QVector3D lo = inv.map(origin);
            const QVector3D ld = inv.mapVector(dir);
            float t;
            if (rayAabb(lo, ld, mn, mx, t) && t > 0.0f && t < bestT) {
                bestT = t;
                best = child;
            }
        }
        pickRecursive(child, origin, dir, best, bestT);
    }
}

SceneObject *RhiRenderer::pick(const QVector3D &origin, const QVector3D &dir) const
{
    SceneObject *best = nullptr;
    float bestT = std::numeric_limits<float>::max();
    pickRecursive(m_sceneRoot, origin, dir, best, bestT);
    return best;
}

void RhiRenderer::render(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const RhiCamera &camera)
{
    if (!m_rhi)
        return;

    // Frame delta for time-accumulated effects (gobo rotation). Clamped so a stall
    // or the first frame doesn't produce a huge jump.
    const double nowSec = m_clock.isValid() ? m_clock.elapsed() / 1000.0 : 0.0;
    m_frameDt = float(qBound(0.0, nowSec - m_lastClockSec, 0.1));
    m_lastClockSec = nowSec;

    QVector<Drawable> drawables;
    QSet<SceneObject *> seenTrusses;
    collectDrawables(m_sceneRoot, drawables, seenTrusses);

    QVector<Drawable> beams;
    collectBeams(m_sceneRoot, beams);

    QVector<Drawable> surfaces;
    collectSurfaces(m_sceneRoot, surfaces);

    const bool volumetric = (m_beamMode == BeamMode::Volumetric);

    // Object buffer layout: [meshes][surfaces][basic beams]. Volumetric beams use a
    // separate param buffer, so they don't take object slots.
    const int surfaceBase = drawables.size();
    const int beamBase = surfaceBase + surfaces.size();
    ensureObjectBuffer(beamBase + (volumetric ? 0 : beams.size()));
    if (volumetric && !beams.isEmpty())
        ensureBeamBuffer(beams.size());

    // Drop geometry for trusses no longer in the scene.
    for (auto it = m_trussMeshes.begin(); it != m_trussMeshes.end(); ) {
        if (!seenTrusses.contains(it.key())) {
            it.value()->release();
            delete it.value();
            m_trussSig.remove(it.key());
            it = m_trussMeshes.erase(it);
        } else {
            ++it;
        }
    }

    QRhiResourceUpdateBatch *u = m_rhi->nextResourceUpdateBatch();

    // Upload gobo texture layers once (or a neutral white layer if none were found).
    if (!m_goboUploaded && m_goboTex) {
        if (!m_goboImages.isEmpty()) {
            QRhiTextureUploadDescription desc;
            QList<QRhiTextureUploadEntry> entries;
            for (int i = 0; i < m_goboImages.size(); ++i)
                entries.append(QRhiTextureUploadEntry(i, 0,
                    QRhiTextureSubresourceUploadDescription(m_goboImages[i])));
            desc.setEntries(entries.cbegin(), entries.cend());
            u->uploadTexture(m_goboTex, desc);
        } else {
            QImage white(kGoboSize, kGoboSize, QImage::Format_RGBA8888);
            white.fill(Qt::white);
            u->uploadTexture(m_goboTex,
                QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0,
                    QRhiTextureSubresourceUploadDescription(white))));
        }
        u->generateMips(m_goboTex);
        m_goboUploaded = true;
    }

    m_grid->upload(m_rhi, u);
    for (const Drawable &d : drawables)
        d.mesh->upload(m_rhi, u);
    if (!beams.isEmpty())
        m_beamCone->upload(m_rhi, u);
    if (!surfaces.isEmpty())
        m_plane->upload(m_rhi, u);

    // Frame constants: clip-space-corrected view-projection + light direction.
    const QMatrix4x4 viewProj =
        m_rhi->clipSpaceCorrMatrix() * camera.projectionMatrix() * camera.viewMatrix();

    float frameData[24];
    std::memcpy(frameData, viewProj.constData(), 16 * sizeof(float));
    const QVector3D lightDir = QVector3D(-0.5f, -1.0f, -0.3f).normalized();
    frameData[16] = lightDir.x();
    frameData[17] = lightDir.y();
    frameData[18] = lightDir.z();
    frameData[19] = float(m_goboIndex);           // gobo selector (spare lightDir.w lane)
    const QVector3D camPos = camera.position();   // for the volumetric beam raymarch
    frameData[20] = camPos.x();
    frameData[21] = camPos.y();
    frameData[22] = camPos.z();
    frameData[23] = float(m_clock.isValid() ? m_clock.elapsed() / 1000.0 : 0.0); // time (camPos.w lane)
    u->updateDynamicBuffer(m_frameBuffer, 0, kFramePayload, frameData);

    // Per-object model + color.
    for (int i = 0; i < drawables.size(); ++i) {
        const Drawable &d = drawables[i];
        float slot[20];
        std::memcpy(slot, d.model.constData(), 16 * sizeof(float));
        slot[16] = float(d.color.redF());
        slot[17] = float(d.color.greenF());
        slot[18] = float(d.color.blueF());
        slot[19] = float(d.color.alphaF());
        u->updateDynamicBuffer(m_objectBuffer, i * m_objectSlotSize, kObjectPayload, slot);
    }

    // Surface model + albedo (object slots after the meshes).
    for (int i = 0; i < surfaces.size(); ++i) {
        const Drawable &d = surfaces[i];
        float slot[20];
        std::memcpy(slot, d.model.constData(), 16 * sizeof(float));
        slot[16] = float(d.color.redF());
        slot[17] = float(d.color.greenF());
        slot[18] = float(d.color.blueF());
        slot[19] = 1.0f;
        u->updateDynamicBuffer(m_objectBuffer, (surfaceBase + i) * m_objectSlotSize,
                               kObjectPayload, slot);
    }

    // Spotlight list for surface lighting, derived from the lit fixtures (same data
    // as the beams). Built every frame regardless of beam mode.
    {
        const int lightCount = qMin(beams.size(), kMaxLights);
        QVector<float> lightData((1 + 5 * kMaxLights) * 4, 0.0f);
        lightData[0] = float(lightCount);
        for (int i = 0; i < lightCount; ++i) {
            QVector3D apex, axis;
            float cosH = 0.0f, length = 0.0f;
            beamConeFromModel(beams[i].model, apex, axis, cosH, length);
            const QColor c = beams[i].color;
            const QColor c2 = beams[i].color2;
            const float cosOuter = cosH;
            // The light reaches well past the visible beam length so its pool lands
            // on floors/walls beyond the cone tip rather than at the falloff edge.
            const float range = qMax(length * 6.0f, 60.0f);

            float *L = lightData.data() + (1 + i * 5) * 4;
            L[0]  = apex.x(); L[1] = apex.y(); L[2] = apex.z(); L[3] = range;
            L[4]  = axis.x(); L[5] = axis.y(); L[6] = axis.z(); L[7] = cosOuter;
            L[8]  = float(c.redF()); L[9] = float(c.greenF()); L[10] = float(c.blueF());
            L[11] = beams[i].gobo;        // gobo pattern index
            L[12] = beams[i].goboRot;     // gobo rotation (radians)
            L[16] = float(c2.redF()); L[17] = float(c2.greenF()); L[18] = float(c2.blueF());
            L[19] = beams[i].split;       // color split position (-1..1, <-1 = none)
        }
        u->updateDynamicBuffer(m_lightsBuffer, 0, kLightsPayload, lightData.constData());
    }

    // Beam uniforms. Basic beams append to the per-object buffer after the meshes
    // and surfaces; volumetric beams write to the dedicated beam buffer.
    if (volumetric) {
        for (int j = 0; j < beams.size(); ++j) {
            const Drawable &d = beams[j];

            QVector3D apex, axis;
            float cosH = 0.0f, length = 0.0f;
            beamConeFromModel(d.model, apex, axis, cosH, length);

            float slot[36] = { 0.0f };
            std::memcpy(slot, d.model.constData(), 16 * sizeof(float));
            slot[16] = float(d.color.redF());
            slot[17] = float(d.color.greenF());
            slot[18] = float(d.color.blueF());
            slot[19] = float(d.color.alphaF());
            slot[20] = apex.x(); slot[21] = apex.y(); slot[22] = apex.z();
            slot[24] = axis.x(); slot[25] = axis.y(); slot[26] = axis.z();
            slot[27] = cosH;
            slot[28] = length;
            slot[29] = d.gobo;      // params.y = gobo pattern index
            slot[30] = d.goboRot;   // params.z = gobo rotation (radians)
            slot[31] = d.split;     // params.w = color split position (-1..1, <-1 = none)
            slot[32] = float(d.color2.redF());
            slot[33] = float(d.color2.greenF());
            slot[34] = float(d.color2.blueF());
            slot[35] = float(d.color2.alphaF());
            u->updateDynamicBuffer(m_beamBuffer, j * m_beamSlotSize, kBeamPayload, slot);
        }
    } else {
        for (int j = 0; j < beams.size(); ++j) {
            const Drawable &d = beams[j];
            float slot[20];
            std::memcpy(slot, d.model.constData(), 16 * sizeof(float));
            slot[16] = float(d.color.redF());
            slot[17] = float(d.color.greenF());
            slot[18] = float(d.color.blueF());
            slot[19] = float(d.color.alphaF());
            u->updateDynamicBuffer(m_objectBuffer, (beamBase + j) * m_objectSlotSize,
                                   kObjectPayload, slot);
        }
    }

    // Gizmo line geometry (world space), rebuilt each frame.
    QByteArray gizmoVerts;
    m_gizmo.buildLines(camera, gizmoVerts);
    if (gizmoVerts.size() > int(kGizmoBytes))
        gizmoVerts.truncate(int(kGizmoBytes));
    const int gizmoVertexCount = gizmoVerts.size() / int(6 * sizeof(float));
    if (gizmoVertexCount > 0)
        u->updateDynamicBuffer(m_gizmoBuffer, 0, gizmoVerts.size(), gizmoVerts.constData());

    const QColor clear(28, 28, 32);
    const QSize pixelSize = rt->pixelSize();

    cb->beginPass(rt, clear, { 1.0f, 0 }, u);
    cb->setViewport(QRhiViewport(0, 0, pixelSize.width(), pixelSize.height()));

    // Ground grid.
    cb->setGraphicsPipeline(m_linePipeline);
    cb->setShaderResources(m_lineSrb);
    const QRhiCommandBuffer::VertexInput gridInput(m_grid->vertexBuffer(), 0);
    cb->setVertexInput(0, 1, &gridInput);
    cb->draw(m_grid->vertexCount());

    // Scene geometry (fixtures share the box mesh; trusses use their own).
    if (!drawables.isEmpty()) {
        cb->setGraphicsPipeline(m_meshPipeline);
        for (int i = 0; i < drawables.size(); ++i) {
            RhiMesh *mesh = drawables[i].mesh;
            if (!mesh->vertexBuffer() || !mesh->isIndexed())
                continue;
            const QRhiCommandBuffer::DynamicOffset off(1, quint32(i) * m_objectSlotSize);
            cb->setShaderResources(m_meshSrb, 1, &off);
            const QRhiCommandBuffer::VertexInput vin(mesh->vertexBuffer(), 0);
            cb->setVertexInput(0, 1, &vin, mesh->indexBuffer(), 0,
                               QRhiCommandBuffer::IndexUInt16);
            cb->drawIndexed(mesh->indexCount());
        }
    }

    // Surfaces (walls / floor): spotlight-lit planes, opaque (writes depth so beams
    // and other geometry occlude correctly).
    if (!surfaces.isEmpty() && m_plane->vertexBuffer() && m_plane->isIndexed()) {
        cb->setGraphicsPipeline(m_surfacePipeline);
        const QRhiCommandBuffer::VertexInput vin(m_plane->vertexBuffer(), 0);
        for (int i = 0; i < surfaces.size(); ++i) {
            const QRhiCommandBuffer::DynamicOffset off(1, quint32(surfaceBase + i) * m_objectSlotSize);
            cb->setShaderResources(m_surfaceSrb, 1, &off);
            cb->setVertexInput(0, 1, &vin, m_plane->indexBuffer(), 0,
                               QRhiCommandBuffer::IndexUInt16);
            cb->drawIndexed(m_plane->indexCount());
        }
    }

    // Light beams: additive over the scene (depth-tested, no depth write). Basic
    // mode draws the flat-alpha cone; volumetric raymarches the cone per fragment.
    if (!beams.isEmpty() && m_beamCone->vertexBuffer() && m_beamCone->isIndexed()) {
        const QRhiCommandBuffer::VertexInput vin(m_beamCone->vertexBuffer(), 0);
        if (volumetric) {
            cb->setGraphicsPipeline(m_beamVolPipeline);
            for (int j = 0; j < beams.size(); ++j) {
                const QRhiCommandBuffer::DynamicOffset off(1, quint32(j) * m_beamSlotSize);
                cb->setShaderResources(m_beamSrb, 1, &off);
                cb->setVertexInput(0, 1, &vin, m_beamCone->indexBuffer(), 0,
                                   QRhiCommandBuffer::IndexUInt16);
                cb->drawIndexed(m_beamCone->indexCount());
            }
        } else {
            cb->setGraphicsPipeline(m_beamPipeline);
            for (int j = 0; j < beams.size(); ++j) {
                const QRhiCommandBuffer::DynamicOffset off(1, quint32(beamBase + j) * m_objectSlotSize);
                cb->setShaderResources(m_meshSrb, 1, &off);
                cb->setVertexInput(0, 1, &vin, m_beamCone->indexBuffer(), 0,
                                   QRhiCommandBuffer::IndexUInt16);
                cb->drawIndexed(m_beamCone->indexCount());
            }
        }
    }

    // Gizmo on top (depth test disabled).
    if (gizmoVertexCount > 0) {
        cb->setGraphicsPipeline(m_gizmoPipeline);
        cb->setShaderResources(m_lineSrb);
        const QRhiCommandBuffer::VertexInput gizmoInput(m_gizmoBuffer, 0);
        cb->setVertexInput(0, 1, &gizmoInput);
        cb->draw(gizmoVertexCount);
    }

    cb->endPass();
}

} // namespace photon
