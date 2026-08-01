#include <algorithm>
#include <rhi/qrhi.h>
#include <QColor>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include "canvassubgraphnode.h"
#include "graph/node/graphcontextnode.h"
#include "canvasoutputnode.h"
#include "canvasrendermanager.h"
#include "canvasdmxsampler.h"
#include "model/graph.h"
#include "model/parameter/parameter.h"
#include "plugin/pluginfactory.h"
#include "graph/parameter/colorparameter.h"
#include "rhi/rhicontext.h"
#include "routine/routineevaluationcontext.h"
#include "processcontext.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "pixel/pixelsourcelayout.h"
#include "pixel/pixelsource.h"

namespace photon {

const QByteArray CanvasSubGraphNode::Width = "width";
const QByteArray CanvasSubGraphNode::Height = "height";
const QByteArray CanvasSubGraphNode::Enabled = "enabled";
const QByteArray CanvasSubGraphNode::Background = "background";
const QByteArray CanvasSubGraphNode::CanvasSubGraphId = "canvas";

keira::NodeInformation CanvasSubGraphNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasSubGraphNode;});
    toReturn.name = "Canvas Graph";
    toReturn.nodeId = "photon.node.canvas-graph";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"bus","surface"};

    return toReturn;
}

CanvasSubGraphNode::CanvasSubGraphNode() : keira::SubGraphNode("photon.node.canvas-graph")
{
    setName("Canvas Graph");

    m_globalsNode = new GraphContextNode;
    m_globalsNode->configure(GraphContextNode::canvasPorts());
    graph()->addNode(m_globalsNode);

    m_outputNode = new CanvasOutputNode;
    m_outputNode->createParameters();
    graph()->addNode(m_outputNode);

    graph()->drainCommandQueue();   // apply the addNodes immediately (see readFromJson)
    graph()->setName("Canvas Graph");
    graph()->setGraphTypeId(CanvasSubGraphId);

    // Register for main-thread rendering. Null in headless tests (no manager) —
    // there evaluate() renders inline.
    m_manager = CanvasRenderManager::instance();
    if (m_manager)
        m_manager->registerCanvas(this);
}

CanvasSubGraphNode::~CanvasSubGraphNode()
{
    // Unregister FIRST: blocks until any in-progress main-thread render of this
    // node finishes, so nothing renders it while we tear down.
    if (m_manager)
        m_manager->unregisterCanvas(this);

    // Globals/Output nodes are owned by the inner graph and freed by ~SubGraphNode.
    releaseSink();
}

void CanvasSubGraphNode::createParameters()
{
    m_widthParam = new keira::IntegerParameter(Width, "Width", 256);
    m_widthParam->setMinimum(1);
    m_widthParam->setMaximum(8192);
    addParameter(m_widthParam);

    m_heightParam = new keira::IntegerParameter(Height, "Height", 256);
    m_heightParam->setMinimum(1);
    m_heightParam->setMaximum(8192);
    addParameter(m_heightParam);

    m_enabledParam = new keira::BooleanParameter(Enabled, "Enabled", true);
    addParameter(m_enabledParam);

    m_backgroundParam = new ColorParameter(Background, "Background", QColor(0, 0, 0, 255));
    addParameter(m_backgroundParam);
}

keira::NodeLibrary *CanvasSubGraphNode::nodeLibrary() const
{
    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    return app ? app->plugins()->nodeLibrary() : nullptr;
}

bool CanvasSubGraphNode::ensureSink(QRhi *rhi, const QSize &size) const
{
    if (m_canvasTexture && m_canvasSize == size)
        return true;

    releaseSink();

    m_canvasTexture = rhi->newTexture(QRhiTexture::RGBA8, size, 1,
                                      QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    if (!m_canvasTexture->create()) {
        qWarning() << "CanvasSubGraphNode: sink texture create failed" << size;
        delete m_canvasTexture;
        m_canvasTexture = nullptr;
        return false;
    }

    QRhiColorAttachment colorAtt(m_canvasTexture);
    QRhiTextureRenderTargetDescription rtDesc(colorAtt);
    m_canvasRT = rhi->newTextureRenderTarget(rtDesc);
    m_canvasRP = m_canvasRT->newCompatibleRenderPassDescriptor();
    m_canvasRT->setRenderPassDescriptor(m_canvasRP);
    if (!m_canvasRT->create()) {
        qWarning() << "CanvasSubGraphNode: sink render target create failed" << size;
        releaseSink();
        return false;
    }

    m_canvasSize = size;
    return true;
}

void CanvasSubGraphNode::releaseSink() const
{
    if (!m_canvasRP && !m_canvasRT && !m_canvasTexture)
        return;

    // QRhi resources must be destroyed on the main thread. If we're not on it
    // (node deleted during worker-thread graph editing), hand the pointers to the
    // main thread to delete — they're detached from the node here, so it's safe
    // even if the node itself is gone by the time the deletion runs.
    if (QThread::isMainThread()) {
        delete m_canvasRP;
        delete m_canvasRT;
        delete m_canvasTexture;
    } else {
        QRhiRenderPassDescriptor *rp = m_canvasRP;
        QRhiTextureRenderTarget *rt = m_canvasRT;
        QRhiTexture *tex = m_canvasTexture;
        QMetaObject::invokeMethod(QCoreApplication::instance(), [rp, rt, tex]() {
            delete rp;
            delete rt;
            delete tex;
        }, Qt::QueuedConnection);
    }

    m_canvasRP = nullptr;
    m_canvasRT = nullptr;
    m_canvasTexture = nullptr;
    m_canvasSize = QSize();
}

void CanvasSubGraphNode::evaluate(keira::EvaluationContext *t_context) const
{
    // Runs on the graph's worker thread. Snapshot inputs and flag a render; the
    // actual GPU work happens on the main thread (CanvasRenderManager). No QRhi
    // calls here — the GL context is main-thread only.
    auto context = static_cast<RoutineEvaluationContext *>(t_context);
    m_relativeTime = context->relativeTime;
    m_globalTime = context->globalTime;
    m_rhiContext = context->rhiContext;   // non-null only in headless tests

    if (m_manager) {
        m_needsRender.store(true);
    } else {
        // No manager (headless tests): render inline on whatever thread called us
        // (expected to be the main thread).
        renderMainThread();
    }

    // Sample DMX on this (worker) thread, from the latest main-thread readback.
    sampleDmx(context);
}

QColor CanvasSubGraphNode::backgroundColor() const
{
    return m_backgroundParam ? m_backgroundParam->value().value<QColor>() : QColor(0, 0, 0, 255);
}

QVector<CanvasOutputNode *> CanvasSubGraphNode::outputNodes() const
{
    QVector<CanvasOutputNode *> outputs;
    for (auto *node : graph()->nodes())
        if (auto *out = dynamic_cast<CanvasOutputNode *>(node))
            outputs << out;
    return outputs;
}

void CanvasSubGraphNode::sampleDmx(RoutineEvaluationContext *context) const
{
    // DMX is driven per Output node: each writes the colours it gathered from its
    // own input texture to the layouts assigned to it.
    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    ProcessContext pc(context->dmxMatrix);
    pc.project = app ? app->project() : nullptr;
    pc.globalTime = context->globalTime;
    pc.relativeTime = context->relativeTime;

    for (auto *output : outputNodes())
        output->writeDmx(pc);
}

void CanvasSubGraphNode::renderMainThread() const
{
    if (!m_enabledParam || !m_enabledParam->value().toBool())
        return;
    if (!m_widthParam || !m_heightParam || !m_backgroundParam || !m_globalsNode || !m_outputNode)
        return;

    // In the app the manager owns the device; headless tests pass it via the context.
    RhiContext *rhiCtx = m_manager ? m_manager->rhiContext() : m_rhiContext;
    if (!rhiCtx || !rhiCtx->isValid())
        return;
    QRhi *rhi = rhiCtx->rhi();

    const QSize size(std::max(1, m_widthParam->value().toInt()),
                     std::max(1, m_heightParam->value().toInt()));
    if (!ensureSink(rhi, size))
        return;

    // The Globals (context) node publishes resolution/time to inner nodes from the
    // inner evaluation context below; exposed graph inputs are relayed by the base
    // SubGraphNode::applyInputs during evaluate().

    QRhiCommandBuffer *cb = nullptr;
    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
        return;

    // Context for the inner graph, carrying the frame's device + command buffer.
    DMXMatrix matrix;
    RoutineEvaluationContext inner(matrix);
    inner.rhiContext = rhiCtx;
    inner.rhiCommandBuffer = cb;
    inner.canvasResolution = size;
    inner.relativeTime = m_relativeTime;
    inner.globalTime = m_globalTime;

    // Clear the sink to the background colour.
    const QColor bg = m_backgroundParam->value().value<QColor>();
    cb->beginPass(m_canvasRT, bg, { 1.0f, 0 });
    cb->endPass();

    // Evaluate the inner graph — producers render into pooled textures and set
    // their output params; the Output node stashes whatever is wired into it.
    SubGraphNode::evaluate(&inner);

    // Composite the auto Output node's texture into the sink (the preview source).
    // Output nodes with assigned layouts already recorded their DMX gather passes
    // during the inner-graph evaluation above.
    const RhiTextureData out = m_outputNode->inputTexture();
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    if (out.texture && out.size == size) {
        QRhiTextureCopyDescription copyDesc;   // whole level 0
        u->copyTexture(m_canvasTexture, out.texture, copyDesc);
    }
    cb->resourceUpdate(u);

    rhi->endOffscreenFrame();   // blocks; Output-node gather readbacks are ready after

    // Pull each Output node's gathered colours for the worker thread to write.
    for (auto *output : outputNodes())
        output->collectGatheredColors();
}

void CanvasSubGraphNode::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    // Drop the auto-created nodes before the serialized graph replaces them, then
    // re-find them by name (mirrors PixelGraph).
    graph()->removeNode(m_globalsNode);
    graph()->removeNode(m_outputNode);
    graph()->drainCommandQueue();
    delete m_globalsNode;
    delete m_outputNode;

    keira::SubGraphNode::readFromJson(t_json, t_library);

    m_globalsNode = dynamic_cast<GraphContextNode *>(graph()->findNode("Globals"));
    m_outputNode = dynamic_cast<CanvasOutputNode *>(graph()->findNode("Output"));
    if (!m_globalsNode || !m_outputNode)
        qWarning() << "CanvasSubGraphNode: could not relink Globals/Output after load";
}

QRhiTexture *CanvasSubGraphNode::outputTexture() const
{
    return m_canvasTexture;
}

QSize CanvasSubGraphNode::canvasSize() const
{
    return m_canvasSize;
}


} // namespace photon
