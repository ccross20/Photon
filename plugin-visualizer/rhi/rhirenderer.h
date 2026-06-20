#ifndef PHOTON_RHIRENDERER_H
#define PHOTON_RHIRENDERER_H

#include <QVector>
#include <QMatrix4x4>
#include <QColor>
#include <QHash>
#include <QByteArray>
#include <QSet>
#include "rhigizmo.h"

class QRhi;
class QRhiBuffer;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;
class QRhiRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiCommandBuffer;

namespace photon {

class SceneObject;
class RhiMesh;
class RhiCamera;

// Owns all QRhi GPU resources for the visualizer and records the frame.
// Milestone A: a ground grid plus one lit box per fixture in the scene tree.
class RhiRenderer
{
public:
    RhiRenderer();
    ~RhiRenderer();

    void setSceneRoot(SceneObject *root) { m_sceneRoot = root; }

    // (Re)creates device resources. Pass a render pass descriptor compatible with
    // the target that will be drawn to (e.g. the swapchain's), and its sample count.
    void initialize(QRhi *rhi, QRhiRenderPassDescriptor *rpDesc, int sampleCount);
    // Releases all device resources (device loss / teardown).
    void releaseResources();

    void render(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const RhiCamera &camera);

    // Ray-pick the nearest selectable scene object (world-space ray). Null = miss.
    SceneObject *pick(const QVector3D &origin, const QVector3D &dir) const;
    void setSelection(SceneObject *obj);
    SceneObject *selection() const { return m_selected; }

    RhiGizmo &gizmo() { return m_gizmo; }

private:
    struct Drawable {
        RhiMesh   *mesh;   // non-owning; shared box or a cached per-truss mesh
        QMatrix4x4 model;
        QColor     color;
    };

    void collectDrawables(SceneObject *obj, QVector<Drawable> &out,
                          QSet<SceneObject *> &seenTrusses);
    RhiMesh *trussMeshFor(SceneObject *truss);
    void ensureObjectBuffer(int count);

    static bool localBounds(SceneObject *obj, QVector3D &outMin, QVector3D &outMax);
    void pickRecursive(SceneObject *obj, const QVector3D &origin, const QVector3D &dir,
                       SceneObject *&best, float &bestT) const;

    QRhi *m_rhi = nullptr;

    RhiMesh *m_box = nullptr;
    RhiMesh *m_grid = nullptr;

    // Per-truss geometry, rebuilt when a truss's parameters change.
    QHash<SceneObject *, RhiMesh *>  m_trussMeshes;
    QHash<SceneObject *, QByteArray> m_trussSig;

    QRhiBuffer *m_frameBuffer = nullptr;   // per-frame constants (binding 0)
    QRhiBuffer *m_objectBuffer = nullptr;  // per-object, dynamic offset (binding 1)
    int         m_objectCapacity = 0;
    quint32     m_objectSlotSize = 0;

    QRhiShaderResourceBindings *m_meshSrb = nullptr;
    QRhiShaderResourceBindings *m_lineSrb = nullptr;
    QRhiGraphicsPipeline *m_meshPipeline = nullptr;
    QRhiGraphicsPipeline *m_linePipeline = nullptr;
    QRhiGraphicsPipeline *m_gizmoPipeline = nullptr;  // lines, depth test off
    QRhiBuffer *m_gizmoBuffer = nullptr;              // dynamic, rebuilt per frame

    RhiGizmo     m_gizmo;
    SceneObject *m_sceneRoot = nullptr;
    SceneObject *m_selected = nullptr;
};

} // namespace photon

#endif // PHOTON_RHIRENDERER_H
