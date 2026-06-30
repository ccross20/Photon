#include <rhi/qrhi.h>
#include <QPlatformSurfaceEvent>
#include <QOffscreenSurface>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include "rhiwindow.h"
#include "rhirenderer.h"
#include "rhigizmo.h"

namespace photon {

namespace {
// Builds a world-space ray from a logical mouse position, accounting for DPR.
void rayFor(const RhiCamera &cam, const QPointF &logicalPos, qreal dpr,
            QVector3D &origin, QVector3D &dir)
{
    cam.rayAt(QPointF(logicalPos.x() * dpr, logicalPos.y() * dpr), origin, dir);
}
} // namespace

RhiWindow::RhiWindow()
{
    setSurfaceType(QSurface::OpenGLSurface);

    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    setFormat(fmt);
}

RhiWindow::~RhiWindow()
{
    releaseRhi();
}

void RhiWindow::setSceneRoot(SceneObject *root)
{
    m_sceneRoot = root;
    if (m_renderer)
        m_renderer->setSceneRoot(root);
}

void RhiWindow::setSelectedSceneObject(SceneObject *obj)
{
    if (!m_renderer || m_renderer->selection() == obj)
        return;
    m_renderer->setSelection(obj);
    requestUpdate();
}

void RhiWindow::setGizmoMode(RhiGizmo::Mode mode)
{
    if (!m_renderer)
        return;
    m_renderer->gizmo().setMode(mode);
    emit gizmoModeChanged(mode);
    requestUpdate();
}

void RhiWindow::setGizmoSpace(RhiGizmo::Space space)
{
    if (!m_renderer)
        return;
    m_renderer->gizmo().setSpace(space);
    requestUpdate();
}

void RhiWindow::setDmxState(const DMXMatrix &dmx)
{
    if (!m_renderer)
        return;
    m_renderer->setDmxState(dmx);
    requestUpdate();
}

void RhiWindow::setBeamMode(RhiRenderer::BeamMode mode)
{
    if (!m_renderer)
        return;
    m_renderer->setBeamMode(mode);
    requestUpdate();
}

void RhiWindow::setGoboIndex(int index)
{
    if (!m_renderer)
        return;
    m_renderer->setGoboIndex(index);
    requestUpdate();
}

int RhiWindow::goboCount() const
{
    return m_renderer ? m_renderer->goboCount() : 0;
}

void RhiWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed() && !m_initialized)
        initRhi();

    const bool exposed = isExposed() && !size().isEmpty();

    // Became exposed again after being hidden — resume rendering.
    if (exposed && m_initialized && m_notExposed) {
        m_notExposed = false;
        renderFrame();
    }

    if (exposed) {
        m_notExposed = false;
        renderFrame();
    } else {
        m_notExposed = true;
    }
}

bool RhiWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::UpdateRequest:
        renderFrame();
        break;
    case QEvent::PlatformSurface:
        // The platform surface is going away — release GPU resources now, while a
        // context can still be made current. This is the deterministic teardown
        // point for a window-hosted QRhi.
        if (static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType()
                == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
            releaseRhi();
        break;
    default:
        break;
    }
    return QWindow::event(e);
}

void RhiWindow::initRhi()
{
    if (m_rhi)
        return;

    m_fallbackSurface = QRhiGles2InitParams::newFallbackSurface(format());

    QRhiGles2InitParams params;
    params.fallbackSurface = m_fallbackSurface;
    params.window = this;
    params.format = format();
    m_rhi = QRhi::create(QRhi::OpenGLES2, &params);
    if (!m_rhi) {
        qWarning("RhiWindow: failed to create QRhi (OpenGL backend)");
        return;
    }

    m_swapChain = m_rhi->newSwapChain();
    m_depthStencil = m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, QSize(),
                                            m_sampleCount, QRhiRenderBuffer::UsedWithSwapChainOnly);
    m_swapChain->setWindow(this);
    m_swapChain->setDepthStencil(m_depthStencil);
    m_swapChain->setSampleCount(m_sampleCount);
    m_renderPass = m_swapChain->newCompatibleRenderPassDescriptor();
    m_swapChain->setRenderPassDescriptor(m_renderPass);
    m_swapChain->createOrResize();

    m_renderer = new RhiRenderer;
    m_renderer->setSceneRoot(m_sceneRoot);
    m_renderer->initialize(m_rhi, m_renderPass, m_sampleCount);

    m_initialized = true;
}

void RhiWindow::releaseRhi()
{
    if (!m_rhi)
        return;

    // Order matters: drop renderer GPU resources, then swapchain objects, then the
    // QRhi itself, then the fallback surface it relied on for context currency.
    if (m_renderer) {
        m_renderer->releaseResources();
        delete m_renderer;
        m_renderer = nullptr;
    }
    delete m_renderPass;    m_renderPass = nullptr;
    delete m_depthStencil;  m_depthStencil = nullptr;
    delete m_swapChain;     m_swapChain = nullptr;
    delete m_rhi;           m_rhi = nullptr;
    delete m_fallbackSurface; m_fallbackSurface = nullptr;

    m_initialized = false;
}

void RhiWindow::renderFrame()
{
    if (!m_rhi || m_notExposed || !isExposed() || size().isEmpty())
        return;

    // Resize the swapchain to match the surface if needed.
    if (m_swapChain->currentPixelSize() != m_swapChain->surfacePixelSize())
        m_swapChain->createOrResize();

    QRhi::FrameOpResult result = m_rhi->beginFrame(m_swapChain);
    if (result == QRhi::FrameOpSwapChainOutOfDate) {
        m_swapChain->createOrResize();
        result = m_rhi->beginFrame(m_swapChain);
    }
    if (result != QRhi::FrameOpSuccess) {
        requestUpdate();
        return;
    }

    QRhiCommandBuffer *cb = m_swapChain->currentFrameCommandBuffer();
    QRhiRenderTarget  *rt = m_swapChain->currentFrameRenderTarget();

    m_camera.setViewportSize(rt->pixelSize());
    m_renderer->render(cb, rt, m_camera);

    m_rhi->endFrame(m_swapChain);

    // Keep the loop live for smooth interaction.
    requestUpdate();
}

void RhiWindow::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->position();
    m_pressPos = event->position();
    m_dragged = false;
    m_gizmoActive = false;

    // Try grabbing a gizmo handle first — that takes priority over camera nav.
    if (event->button() == Qt::LeftButton && m_renderer) {
        QVector3D o, d;
        rayFor(m_camera, event->position(), devicePixelRatio(), o, d);
        if (m_renderer->gizmo().beginDrag(o, d, m_camera)) {
            m_gizmoActive = true;
            return;
        }
    }

    if (event->button() == Qt::LeftButton) {
        const bool shift = event->modifiers() & Qt::ShiftModifier;
        m_panning = shift;
        m_orbiting = !shift;
    } else if (event->button() == Qt::MiddleButton) {
        m_panning = true;
    }
}

void RhiWindow::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();
    const float dx = float(pos.x() - m_lastPos.x());
    const float dy = float(pos.y() - m_lastPos.y());
    m_lastPos = pos;

    if ((pos - m_pressPos).manhattanLength() > 3.0)
        m_dragged = true;

    if (m_gizmoActive && m_renderer) {
        QVector3D o, d;
        rayFor(m_camera, pos, devicePixelRatio(), o, d);
        m_renderer->gizmo().updateDrag(o, d);
        requestUpdate();
        return;
    }

    if (m_orbiting)
        m_camera.orbit(dx, dy);
    else if (m_panning)
        m_camera.pan(dx, dy);

    if (m_orbiting || m_panning)
        requestUpdate();
}

void RhiWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_gizmoActive && m_renderer) {
        m_renderer->gizmo().endDrag();
        m_gizmoActive = false;
        requestUpdate();
    } else if (event->button() == Qt::LeftButton && !m_dragged && m_renderer) {
        // A left click that did not drag is a selection.
        QVector3D origin, dir;
        rayFor(m_camera, event->position(), devicePixelRatio(), origin, dir);
        m_renderer->setSelection(m_renderer->pick(origin, dir));
        emit selectionChanged(m_renderer->selection());
        requestUpdate();
    }

    m_orbiting = false;
    m_panning = false;
}

void RhiWindow::keyPressEvent(QKeyEvent *event)
{
    if (!m_renderer) {
        QWindow::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_W: setGizmoMode(RhiGizmo::Translate); break;
    case Qt::Key_E: setGizmoMode(RhiGizmo::Rotate);    break;
    case Qt::Key_R: setGizmoMode(RhiGizmo::Scale);     break;
    case Qt::Key_Q: setGizmoMode(RhiGizmo::None);      break;
    case Qt::Key_Escape:
        m_renderer->setSelection(nullptr);
        emit selectionChanged(nullptr);
        setGizmoMode(RhiGizmo::None);
        break;
    default:
        QWindow::keyPressEvent(event);
        break;
    }
}

void RhiWindow::wheelEvent(QWheelEvent *event)
{
    m_camera.zoom(event->angleDelta().y() / 120.0f);
    requestUpdate();
}

} // namespace photon
