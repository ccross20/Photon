#include <rhi/qrhi.h>
#include <QFile>
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
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/dimmercapability.h"
#include "fixture/capability/pancapability.h"
#include "fixture/capability/tiltcapability.h"
#include "fixture/capability/anglecapability.h"

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

constexpr quint32 kFramePayload  = 20 * sizeof(float); // mat4 + vec4
constexpr quint32 kObjectPayload = 20 * sizeof(float); // mat4 + vec4
constexpr quint32 kGizmoBytes    = 4096 * 6 * sizeof(float); // dynamic line verts

// Beam appearance.
constexpr float kBeamLength    = 6.0f;    // reference throw (world units) at the reference angle
constexpr float kBeamHalfAngle = 8.0f;    // default/reference cone half-angle, degrees
constexpr float kBeamGain      = 0.55f;   // overall additive strength
constexpr float kBeamMinLevel  = 0.02f;   // skip beams dimmer than this
constexpr float kBeamThrowMin  = 0.5f;    // throw scale clamp (wide washes)
constexpr float kBeamThrowMax  = 2.0f;    // throw scale clamp (tight spots)

} // namespace

RhiRenderer::RhiRenderer() {}

RhiRenderer::~RhiRenderer()
{
    releaseResources();
    delete m_box;
    delete m_grid;
    delete m_beamCone;
}

void RhiRenderer::releaseResources()
{
    delete m_meshPipeline;  m_meshPipeline = nullptr;
    delete m_linePipeline;  m_linePipeline = nullptr;
    delete m_beamPipeline;  m_beamPipeline = nullptr;
    delete m_gizmoPipeline; m_gizmoPipeline = nullptr;
    delete m_meshSrb;       m_meshSrb = nullptr;
    delete m_lineSrb;       m_lineSrb = nullptr;
    delete m_frameBuffer;   m_frameBuffer = nullptr;
    delete m_objectBuffer;  m_objectBuffer = nullptr;
    delete m_gizmoBuffer;   m_gizmoBuffer = nullptr;
    m_objectCapacity = 0;

    if (m_box)      m_box->release();
    if (m_grid)     m_grid->release();
    if (m_beamCone) m_beamCone->release();

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
}

void RhiRenderer::initialize(QRhi *rhi, QRhiRenderPassDescriptor *rpDesc, int sampleCount)
{
    m_rhi = rhi;

    if (!m_box)
        m_box = RhiMesh::createBox(0.15f, 0.15f, 0.15f);
    if (!m_grid)
        m_grid = RhiMesh::createGrid(20, 1.0f);
    if (!m_beamCone)
        m_beamCone = RhiMesh::createCone(28);

    // Per-object slot stride must satisfy the uniform-buffer offset alignment.
    m_objectSlotSize = m_rhi->ubufAlignment();
    while (m_objectSlotSize < kObjectPayload)
        m_objectSlotSize += m_rhi->ubufAlignment();

    m_frameBuffer = m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, kFramePayload);
    m_frameBuffer->create();

    // Creates m_objectBuffer and m_meshSrb.
    ensureObjectBuffer(64);

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

void RhiRenderer::collectBeams(SceneObject *obj, QVector<Drawable> &out) const
{
    if (!obj || !m_beamCone)
        return;

    const float refTan = std::tan(qDegreesToRadians(kBeamHalfAngle));

    for (SceneObject *child : obj->sceneChildren()) {
        if (child->typeId() == "fixture") {
            auto *fix = static_cast<Fixture *>(child);
            QColor emitted;
            float intensity = 0.0f;
            if (evaluateFixture(fix, emitted, intensity) && intensity > kBeamMinLevel) {
                // Pan/tilt aim the beam relative to the fixture's own transform.
                // Centered so 50% DMX = beam straight down the fixture's local -Y.
                float panDeg = 0.0f, tiltDeg = 0.0f;
                const auto pans = fix->findCapability(Capability_Pan);
                if (!pans.isEmpty()) {
                    auto *p = static_cast<PanCapability *>(pans.first());
                    const float range = float(p->angleEnd() - p->angleStart());
                    panDeg = float(p->getAnglePercent(m_dmx)) * range - range * 0.5f;
                }
                const auto tilts = fix->findCapability(Capability_Tilt);
                if (!tilts.isEmpty()) {
                    auto *t = static_cast<TiltCapability *>(tilts.first());
                    const float range = float(t->angleEnd() - t->angleStart());
                    tiltDeg = float(t->getAnglePercent(m_dmx)) * range - range * 0.5f;
                }

                // Beam width follows zoom; tighter beams throw farther (clamped).
                const float halfAngle = beamHalfAngleFor(fix);
                const float tanHalf = std::tan(qDegreesToRadians(halfAngle));
                const float length = kBeamLength
                    * qBound(kBeamThrowMin, refTan / tanHalf, kBeamThrowMax);
                const float baseRadius = length * tanHalf;

                // Unit cone (apex at origin, axis -Y) oriented and sized into place.
                QMatrix4x4 m = fix->globalMatrix();
                m.rotate(panDeg, 0.0f, 1.0f, 0.0f);
                m.rotate(tiltDeg, 1.0f, 0.0f, 0.0f);
                m.scale(baseRadius, length, baseRadius);

                out.append({ m_beamCone, m,
                             QColor::fromRgbF(float(emitted.redF()), float(emitted.greenF()),
                                              float(emitted.blueF()), kBeamGain) });
            }
        }
        collectBeams(child, out);
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

    QVector<Drawable> drawables;
    QSet<SceneObject *> seenTrusses;
    collectDrawables(m_sceneRoot, drawables, seenTrusses);

    QVector<Drawable> beams;
    collectBeams(m_sceneRoot, beams);

    ensureObjectBuffer(drawables.size() + beams.size());

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

    m_grid->upload(m_rhi, u);
    for (const Drawable &d : drawables)
        d.mesh->upload(m_rhi, u);
    if (!beams.isEmpty())
        m_beamCone->upload(m_rhi, u);

    // Frame constants: clip-space-corrected view-projection + light direction.
    const QMatrix4x4 viewProj =
        m_rhi->clipSpaceCorrMatrix() * camera.projectionMatrix() * camera.viewMatrix();

    float frameData[20];
    std::memcpy(frameData, viewProj.constData(), 16 * sizeof(float));
    const QVector3D lightDir = QVector3D(-0.5f, -1.0f, -0.3f).normalized();
    frameData[16] = lightDir.x();
    frameData[17] = lightDir.y();
    frameData[18] = lightDir.z();
    frameData[19] = 0.0f;
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

    // Beam slots follow the mesh slots in the same buffer.
    const int beamBase = drawables.size();
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

    // Light beams: additive cones over the scene (depth-tested, no depth write).
    if (!beams.isEmpty() && m_beamCone->vertexBuffer() && m_beamCone->isIndexed()) {
        cb->setGraphicsPipeline(m_beamPipeline);
        const QRhiCommandBuffer::VertexInput vin(m_beamCone->vertexBuffer(), 0);
        for (int j = 0; j < beams.size(); ++j) {
            const QRhiCommandBuffer::DynamicOffset off(1, quint32(beamBase + j) * m_objectSlotSize);
            cb->setShaderResources(m_meshSrb, 1, &off);
            cb->setVertexInput(0, 1, &vin, m_beamCone->indexBuffer(), 0,
                               QRhiCommandBuffer::IndexUInt16);
            cb->drawIndexed(m_beamCone->indexCount());
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
