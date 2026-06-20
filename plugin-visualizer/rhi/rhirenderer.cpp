#include <rhi/qrhi.h>
#include <QFile>
#include <QDataStream>
#include <cstring>
#include <limits>
#include <utility>
#include "rhirenderer.h"
#include "rhimesh.h"
#include "rhicamera.h"
#include "scene/sceneobject.h"
#include "scene/truss.h"

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

} // namespace

RhiRenderer::RhiRenderer() {}

RhiRenderer::~RhiRenderer()
{
    releaseResources();
    delete m_box;
    delete m_grid;
}

void RhiRenderer::releaseResources()
{
    delete m_meshPipeline;  m_meshPipeline = nullptr;
    delete m_linePipeline;  m_linePipeline = nullptr;
    delete m_gizmoPipeline; m_gizmoPipeline = nullptr;
    delete m_meshSrb;       m_meshSrb = nullptr;
    delete m_lineSrb;       m_lineSrb = nullptr;
    delete m_frameBuffer;   m_frameBuffer = nullptr;
    delete m_objectBuffer;  m_objectBuffer = nullptr;
    delete m_gizmoBuffer;   m_gizmoBuffer = nullptr;
    m_objectCapacity = 0;

    if (m_box)  m_box->release();
    if (m_grid) m_grid->release();

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
            out.append({ m_box, child->globalMatrix(), sel ? highlight : QColor(205, 205, 215) });
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
    ensureObjectBuffer(drawables.size());

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
