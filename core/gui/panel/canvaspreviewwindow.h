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

class CanvasOutputNode;
class CanvasLayerGroup;
class CanvasRenderManager;

// What the preview window currently shows: either a canvas graph's Output node
// (outputNode set) or a CanvasLayerGroup's own sink (layerGroup set). At most
// one should be set at a time.
struct CanvasPreviewTarget
{
    CanvasOutputNode *outputNode = nullptr;
    CanvasLayerGroup *layerGroup = nullptr;

    bool operator==(const CanvasPreviewTarget &o) const
    {
        return outputNode == o.outputNode && layerGroup == o.layerGroup;
    }
    bool operator!=(const CanvasPreviewTarget &o) const { return !(*this == o); }
    bool isNull() const { return !outputNode && !layerGroup; }
};

// A QWindow that displays a canvas Output node's or CanvasLayerGroup's texture.
// It owns its own QRhi (OpenGL, in the global share group like the visualizer's
// RhiWindow), so it can import the texture by its shared GL id — no CPU
// roundtrip — and draw it fullscreen. Embedded in CanvasPreviewPanel via
// createWindowContainer. See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasPreviewWindow : public QWindow
{
    Q_OBJECT
public:
    CanvasPreviewWindow();
    ~CanvasPreviewWindow() override;

    // Which target to show (validated against the render manager each frame).
    void setTarget(const CanvasPreviewTarget &target);

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

    CanvasPreviewTarget  m_target;
    CanvasRenderManager *m_manager = nullptr;

    bool m_initialized = false;
    bool m_notExposed = true;
};

} // namespace photon

#endif // PHOTON_CANVASPREVIEWWINDOW_H
