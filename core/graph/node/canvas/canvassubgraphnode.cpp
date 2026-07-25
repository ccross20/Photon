#include <algorithm>
#include <rhi/qrhi.h>
#include <QColor>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include "canvassubgraphnode.h"
#include "canvasglobalsnode.h"
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
const QByteArray CanvasSubGraphNode::DmxOutput = "dmxOutput";
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

    m_globalsNode = new CanvasGlobalsNode;
    m_globalsNode->createParameters();
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
    delete m_dmxSampler;   // its dtor defers GPU teardown to the main thread if needed
    m_dmxSampler = nullptr;
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

    m_dmxOutputParam = new keira::BooleanParameter(DmxOutput, "DMX Output", false);
    addParameter(m_dmxOutputParam);

    m_backgroundParam = new ColorParameter(Background, "Background", QColor(0, 0, 0, 255));
    addParameter(m_backgroundParam);
}

bool CanvasSubGraphNode::isBuiltInParam(keira::Parameter *t_param) const
{
    return t_param == m_widthParam || t_param == m_heightParam || t_param == m_enabledParam
        || t_param == m_dmxOutputParam || t_param == m_backgroundParam;
}

void CanvasSubGraphNode::parameterWasAdded(keira::Parameter *t_param)
{
    if (isBuiltInParam(t_param) || !m_globalsNode)
        return;

    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    if (!app)   // headless: no node library to clone through
        return;

    // Mirror the added parameter onto the Globals node as an output that inner
    // nodes can wire from; its value is relayed each frame in renderMainThread().
    auto *clone = t_param->clone(app->plugins()->nodeLibrary());
    clone->setConnectionOptions(keira::AllowMultipleOutput);
    m_globalsNode->addParameter(clone);
    graph()->drainCommandQueue();

    m_globalsParams.append(t_param);
    m_passThroughParams.append(clone);
}

void CanvasSubGraphNode::parameterWasRemoved(keira::Parameter *)
{
    // Matches FixtureSubGraphNode/PixelGraph, which don't prune the Globals clone.
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

void CanvasSubGraphNode::sampleDmx(RoutineEvaluationContext *context) const
{
    if (!m_dmxOutputParam || !m_dmxOutputParam->value().toBool())
        return;

    // qobject_cast (not the photonApp macro) so headless tests, where qApp is a
    // plain QApplication, safely get null instead of a bad static_cast.
    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    if (!app || !app->project())
        return;

    QVector<QColor> colors;
    {
        QMutexLocker lock(&m_gatheredMutex);
        colors = m_gatheredColors;   // implicitly shared; cheap copy
    }
    if (colors.isEmpty())
        return;

    ProcessContext pc(context->dmxMatrix);
    pc.gatheredColors = &colors;
    pc.gatheredIndex = 0;
    pc.project = app->project();
    pc.globalTime = context->globalTime;
    pc.relativeTime = context->relativeTime;

    // Iterate layouts in the SAME order buildSampleUVs() did, so each source pops
    // the colours gathered for its own positions.
    // NOTE: samples every project pixel layout — per-canvas selection is a follow-up.
    for (auto *layout : app->project()->pixelLayouts()->layouts())
        layout->process(pc);
}

QVector<QPointF> CanvasSubGraphNode::buildSampleUVs() const
{
    QVector<QPointF> uvs;
    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    if (!app || !app->project())
        return uvs;

    for (auto *layout : app->project()->pixelLayouts()->layouts())
        for (auto *sourceLayout : layout->sourceLayouts())
            if (sourceLayout->source())
                sourceLayout->source()->collectSampleUVs(uvs, sourceLayout->transform());

    return uvs;
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

    // Push per-frame constants onto the Globals node for inner nodes to read.
    m_globalsNode->setValue(CanvasGlobalsNode::WidthParam, size.width());
    m_globalsNode->setValue(CanvasGlobalsNode::HeightParam, size.height());
    m_globalsNode->setValue(CanvasGlobalsNode::TimeParam, m_relativeTime);
    m_globalsNode->setValue(CanvasGlobalsNode::GlobalTimeParam, m_globalTime);

    // Relay pass-through parameters (values wired into this node from the outer
    // graph) onto their Globals-node clones for inner nodes to read.
    for (int i = 0; i < m_passThroughParams.size(); ++i)
        m_passThroughParams[i]->setValue(m_globalsParams[i]->value());

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

    // Composite the Output node's texture into the sink. When DMX output is on,
    // gather just the pixel-sample colours on the GPU (5b) instead of reading the
    // whole canvas back.
    const bool dmxOut = m_dmxOutputParam && m_dmxOutputParam->value().toBool();
    const RhiTextureData out = m_outputNode->inputTexture();

    QVector<QPointF> uvs;
    if (dmxOut)
        uvs = buildSampleUVs();

    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    if (out.texture && out.size == size) {
        QRhiTextureCopyDescription copyDesc;   // whole level 0
        u->copyTexture(m_canvasTexture, out.texture, copyDesc);
    }

    if (dmxOut && !uvs.isEmpty()) {
        if (!m_dmxSampler)
            m_dmxSampler = new CanvasDmxSampler;
        m_dmxSampler->recordGather(rhi, cb, m_canvasTexture, uvs, u);   // applies u
    } else {
        cb->resourceUpdate(u);
    }

    rhi->endOffscreenFrame();   // blocks; the gather readback is ready afterwards

    if (dmxOut && !uvs.isEmpty() && m_dmxSampler) {
        QVector<QColor> colors;
        if (m_dmxSampler->takeColors(colors)) {
            QMutexLocker lock(&m_gatheredMutex);
            m_gatheredColors.swap(colors);
        }
    }
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

    m_globalsNode = dynamic_cast<CanvasGlobalsNode *>(graph()->findNode("Globals"));
    m_outputNode = dynamic_cast<CanvasOutputNode *>(graph()->findNode("Output"));
    if (!m_globalsNode || !m_outputNode)
        qWarning() << "CanvasSubGraphNode: could not relink Globals/Output after load";

    // Relink pass-through parameters to their Globals-node clones (both were
    // restored by the load) by matching id.
    m_globalsParams.clear();
    m_passThroughParams.clear();
    for (auto *param : parameters()) {
        if (isBuiltInParam(param))
            continue;
        auto *nodeParam = m_globalsNode ? m_globalsNode->findParameter(param->id()) : nullptr;
        if (nodeParam) {
            m_globalsParams.append(param);
            m_passThroughParams.append(nodeParam);
        } else {
            qWarning() << "CanvasSubGraphNode: could not relink pass-through param" << param->id();
        }
    }
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
