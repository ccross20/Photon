#include <rhi/qrhi.h>
#include <QFile>
#include <QPlatformSurfaceEvent>
#include <QOffscreenSurface>
#include <QDebug>
#include "canvaspreviewwindow.h"
#include "graph/node/canvas/canvassubgraphnode.h"
#include "graph/node/canvas/canvasrendermanager.h"

namespace photon {

namespace {
QShader loadShader(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());
    qWarning() << "CanvasPreviewWindow: failed to load shader" << path;
    return QShader();
}
}

CanvasPreviewWindow::CanvasPreviewWindow()
{
    setSurfaceType(QSurface::OpenGLSurface);

    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    setFormat(fmt);

    m_manager = CanvasRenderManager::instance();
}

CanvasPreviewWindow::~CanvasPreviewWindow()
{
    releaseRhi();
}

void CanvasPreviewWindow::setCanvas(CanvasSubGraphNode *node)
{
    m_node = node;
    requestUpdate();
}

void CanvasPreviewWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed() && !m_initialized)
        initRhi();

    const bool exposed = isExposed() && !size().isEmpty();
    if (exposed && m_initialized) {
        m_notExposed = false;
        renderFrame();
    } else {
        m_notExposed = true;
    }
}

bool CanvasPreviewWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::UpdateRequest:
        renderFrame();
        break;
    case QEvent::PlatformSurface:
        if (static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType()
                == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
            releaseRhi();
        break;
    default:
        break;
    }
    return QWindow::event(e);
}

void CanvasPreviewWindow::initRhi()
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
        qWarning() << "CanvasPreviewWindow: failed to create QRhi";
        return;
    }

    m_swapChain = m_rhi->newSwapChain();
    m_swapChain->setWindow(this);
    m_swapChain->setSampleCount(1);
    m_renderPass = m_swapChain->newCompatibleRenderPassDescriptor();
    m_swapChain->setRenderPassDescriptor(m_renderPass);
    m_swapChain->createOrResize();

    m_sampler = m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                  QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
    m_sampler->create();

    m_initialized = true;
}

void CanvasPreviewWindow::releaseRhi()
{
    if (!m_rhi)
        return;

    delete m_imported;   m_imported = nullptr;
    delete m_pipeline;   m_pipeline = nullptr;
    delete m_srb;        m_srb = nullptr;
    delete m_sampler;    m_sampler = nullptr;
    delete m_renderPass; m_renderPass = nullptr;
    delete m_swapChain;  m_swapChain = nullptr;
    delete m_rhi;        m_rhi = nullptr;
    delete m_fallbackSurface; m_fallbackSurface = nullptr;

    m_importedId = 0;
    m_importedSize = QSize();
    m_initialized = false;
}

bool CanvasPreviewWindow::ensurePipeline()
{
    if (m_pipeline)
        return true;
    if (!m_srb)   // needs the imported texture bound first
        return false;

    const QShader vert = loadShader(QStringLiteral(":/canvas/shaders/fullscreen.vert.qsb"));
    const QShader frag = loadShader(QStringLiteral(":/canvas/shaders/blit.frag.qsb"));
    if (!vert.isValid() || !frag.isValid())
        return false;

    m_pipeline = m_rhi->newGraphicsPipeline();
    m_pipeline->setShaderStages({
        { QRhiShaderStage::Vertex, vert },
        { QRhiShaderStage::Fragment, frag }
    });
    m_pipeline->setVertexInputLayout(QRhiVertexInputLayout());
    m_pipeline->setShaderResourceBindings(m_srb);
    m_pipeline->setRenderPassDescriptor(m_renderPass);
    m_pipeline->setSampleCount(1);
    m_pipeline->setDepthTest(false);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_pipeline->setTopology(QRhiGraphicsPipeline::Triangles);
    return m_pipeline->create();
}

void CanvasPreviewWindow::renderFrame()
{
    if (!m_rhi || m_notExposed || !isExposed() || size().isEmpty())
        return;

    if (m_swapChain->currentPixelSize() != m_swapChain->surfacePixelSize())
        m_swapChain->createOrResize();

    QRhi::FrameOpResult r = m_rhi->beginFrame(m_swapChain);
    if (r == QRhi::FrameOpSwapChainOutOfDate) {
        m_swapChain->createOrResize();
        r = m_rhi->beginFrame(m_swapChain);
    }
    if (r != QRhi::FrameOpSuccess) {
        requestUpdate();
        return;
    }

    // Resolve the canvas texture to show (validate the node is still live).
    QRhiTexture *canvasTex = nullptr;
    QSize canvasSize;
    if (m_node && (!m_manager || m_manager->isRegistered(m_node))) {
        canvasTex = m_node->outputTexture();
        canvasSize = m_node->canvasSize();
    }

    // (Re)import the canvas sink texture by its shared GL id when it changes.
    if (canvasTex && !canvasSize.isEmpty()) {
        const quint64 id = canvasTex->nativeTexture().object;
        if (id != m_importedId || canvasSize != m_importedSize || !m_imported) {
            delete m_imported;
            m_imported = m_rhi->newTexture(QRhiTexture::RGBA8, canvasSize, 1);
            if (m_imported->createFrom({ id, 0 })) {
                delete m_srb;
                m_srb = m_rhi->newShaderResourceBindings();
                m_srb->setBindings({
                    QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage,
                                                              m_imported, m_sampler)
                });
                m_srb->create();
                m_importedId = id;
                m_importedSize = canvasSize;
            } else {
                delete m_imported;
                m_imported = nullptr;
            }
        }
    }

    QRhiCommandBuffer *cb = m_swapChain->currentFrameCommandBuffer();
    QRhiRenderTarget  *rt = m_swapChain->currentFrameRenderTarget();

    const QColor clear(24, 24, 28);   // dark backdrop when nothing to show
    cb->beginPass(rt, clear, { 1.0f, 0 });
    if (m_imported && ensurePipeline()) {
        const QSize px = rt->pixelSize();
        cb->setGraphicsPipeline(m_pipeline);
        cb->setViewport(QRhiViewport(0, 0, px.width(), px.height()));
        cb->setShaderResources(m_srb);
        cb->draw(3);
    }
    cb->endPass();

    m_rhi->endFrame(m_swapChain);

    requestUpdate();   // keep the preview live
}

} // namespace photon
