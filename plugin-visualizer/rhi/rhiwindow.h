#ifndef PHOTON_RHIWINDOW_H
#define PHOTON_RHIWINDOW_H

#include <QWindow>
#include <QPointF>
#include "rhicamera.h"
#include "rhigizmo.h"

class QRhi;
class QRhiSwapChain;
class QRhiRenderBuffer;
class QRhiRenderPassDescriptor;
class QOffscreenSurface;

namespace photon {

class SceneObject;
class RhiRenderer;
class DMXMatrix;

// A QWindow that owns its QRhi, swapchain and renderer outright. Embedded into the
// widget hierarchy via QWidget::createWindowContainer (see RhiViewport).
//
// We manage the QRhi lifetime ourselves — created on first expose, released either
// when the platform surface is about to be destroyed or in the destructor (with the
// QRhi's fallback offscreen surface keeping a context available for cleanup). This
// avoids QRhiWidget, whose destructor dereferences an already-freed QRhi when an
// embedded instance's top-level is destroyed (panel close).
class RhiWindow : public QWindow
{
    Q_OBJECT
public:
    RhiWindow();
    ~RhiWindow() override;

    void setSceneRoot(SceneObject *root);
    void setSelectedSceneObject(SceneObject *obj);
    void setGizmoMode(RhiGizmo::Mode mode);
    void setGizmoSpace(RhiGizmo::Space space);
    void setDmxState(const DMXMatrix &dmx);

signals:
    void selectionChanged(photon::SceneObject *obj);
    void gizmoModeChanged(RhiGizmo::Mode mode);

protected:
    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    void initRhi();
    void releaseRhi();
    void renderFrame();

    QRhi                     *m_rhi = nullptr;
    QOffscreenSurface        *m_fallbackSurface = nullptr;
    QRhiSwapChain            *m_swapChain = nullptr;
    QRhiRenderBuffer         *m_depthStencil = nullptr;
    QRhiRenderPassDescriptor *m_renderPass = nullptr;
    RhiRenderer              *m_renderer = nullptr;

    RhiCamera    m_camera;
    SceneObject *m_sceneRoot = nullptr;

    bool m_initialized = false;
    bool m_notExposed = true;
    int  m_sampleCount = 4;

    QPointF m_lastPos;
    QPointF m_pressPos;
    bool    m_orbiting = false;
    bool    m_panning = false;
    bool    m_dragged = false;
    bool    m_gizmoActive = false;
};

} // namespace photon

#endif // PHOTON_RHIWINDOW_H
