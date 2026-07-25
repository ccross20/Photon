#ifndef PHOTON_CANVASPREVIEWWINDOW_H
#define PHOTON_CANVASPREVIEWWINDOW_H

#include <QWindow>
#include <QSize>
#include "photon-global.h"

class QRhi;
class QRhiSwapChain;
class QRhiRenderPassDescriptor;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;
class QRhiSampler;
class QRhiTexture;
class QOffscreenSurface;

namespace photon {

class CanvasSubGraphNode;
class CanvasRenderManager;

// A QWindow that displays a canvas subgraph's sink texture. It owns its own QRhi
// (OpenGL, in the global share group like the visualizer's RhiWindow), so it can
// import the canvas's sink texture by its shared GL id — no CPU roundtrip — and
// draw it fullscreen. Embedded in CanvasPreviewPanel via createWindowContainer.
// See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasPreviewWindow : public QWindow
{
    Q_OBJECT
public:
    CanvasPreviewWindow();
    ~CanvasPreviewWindow() override;

    // Which canvas to show (validated against the render manager each frame).
    void setCanvas(CanvasSubGraphNode *node);

protected:
    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *) override;

private:
    void initRhi();
    void releaseRhi();
    void renderFrame();
    bool ensurePipeline();

    QRhi                     *m_rhi = nullptr;
    QOffscreenSurface        *m_fallbackSurface = nullptr;
    QRhiSwapChain            *m_swapChain = nullptr;
    QRhiRenderPassDescriptor *m_renderPass = nullptr;
    QRhiGraphicsPipeline     *m_pipeline = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiSampler              *m_sampler = nullptr;
    QRhiTexture              *m_imported = nullptr;   // wraps the canvas sink's GL id

    quint64 m_importedId = 0;
    QSize   m_importedSize;

    CanvasSubGraphNode  *m_node = nullptr;
    CanvasRenderManager *m_manager = nullptr;

    bool m_initialized = false;
    bool m_notExposed = true;
};

} // namespace photon

#endif // PHOTON_CANVASPREVIEWWINDOW_H
