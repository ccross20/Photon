#include "canvasclip.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "canvaslayergroup.h"
#include "processcontext.h"
#include "model/graph.h"
#include "graph/node/graphcontextnode.h"
#include "graph/node/canvas/canvasoutputnode.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

class CanvasClip::Impl
{
public:
    void createGraph();

    keira::Graph *graph = nullptr;
    GraphContextNode *globalsNode = nullptr;
    CanvasOutputNode *outputNode = nullptr;

    // The owning group, cached on layerDidChange() rather than resolved via
    // layer()->parentGroup() at use time - by the time ~CanvasClip() runs,
    // layer() has already been cleared (Clip::Impl::setLayer(nullptr) runs
    // before the clip's own destructor), so this is the only way to reach the
    // group during teardown to purge/block against an in-flight render.
    CanvasLayerGroup *cachedGroup = nullptr;
};

void CanvasClip::Impl::createGraph()
{
    graph = new keira::Graph;
    graph->setGraphTypeId("canvas");
    graph->setName("Canvas Clip Graph");

    globalsNode = new GraphContextNode;
    globalsNode->configure(GraphContextNode::canvasPorts());
    graph->addNode(globalsNode);

    outputNode = new CanvasOutputNode;
    outputNode->createParameters();
    graph->addNode(outputNode);

    graph->drainCommandQueue();   // apply the addNodes immediately (see readFromJson)
}

CanvasClip::CanvasClip(): Clip(), m_impl(new Impl)
{
    setId("canvasclip");
    m_impl->createGraph();
    addChannelParameter(new Point2ChannelParameter("position",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{1,1}));
    addChannelParameter(new NumberChannelParameter("rotation"));
}

CanvasClip::CanvasClip(double t_start, double t_duration, QObject *t_parent) : Clip(t_start, t_duration, t_parent), m_impl(new Impl)
{
    setId("canvasclip");
    m_impl->createGraph();
    addChannelParameter(new Point2ChannelParameter("position",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{1,1}));
    addChannelParameter(new NumberChannelParameter("rotation"));
}

CanvasClip::~CanvasClip()
{
    // Block until any in-progress CanvasLayerGroup::renderMainThread() that
    // might be evaluating this clip's graph (on the main thread, possibly up
    // to ~16ms after processChannels() last queued it from the eval thread)
    // has finished, and purge any queued reference to this clip - otherwise
    // the render could dereference/evaluate a graph that's about to be freed
    // below. No-op if the group is itself mid-teardown (it already guarantees
    // no render can be in flight in that case) or if this clip was never
    // attached to a group.
    if(m_impl->cachedGroup)
        m_impl->cachedGroup->clipBeingDestroyed(this);

    delete m_impl->graph;   // owns globalsNode/outputNode
    delete m_impl;
}

keira::Graph *CanvasClip::contentGraph() const
{
    return m_impl->graph;
}

void CanvasClip::layerDidChange(Layer *t_layer)
{
    Clip::layerDidChange(t_layer);
    if(t_layer)
        m_impl->cachedGroup = dynamic_cast<CanvasLayerGroup*>(t_layer->parentGroup());
    // else: keep the last-known group - needed by ~CanvasClip() above.
}

void CanvasClip::processChannels(ProcessContext &t_context)
{
    if(!m_impl->cachedGroup)
        return;

    CanvasClipRenderState state;
    state.clip = this;
    state.position = t_context.channelValues.value("position").toPointF();
    state.center = t_context.channelValues.value("center").toPointF();
    state.scale = t_context.channelValues.value("scale").toPointF();
    state.rotation = t_context.channelValues.value("rotation").toDouble();
    state.strength = strengthAtTime(t_context.relativeTime);
    state.relativeTime = t_context.relativeTime;
    state.globalTime = t_context.globalTime;

    m_impl->cachedGroup->queueClipForRender(state);
}

void CanvasClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void CanvasClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);

    if(t_json.contains("graph"))
    {
        delete m_impl->graph;   // owns the previous globalsNode/outputNode
        m_impl->graph = new keira::Graph;
        m_impl->graph->setGraphTypeId("canvas");
        m_impl->graph->readFromJson(t_json.value("graph").toObject(), photonApp->plugins()->nodeLibrary());
        m_impl->globalsNode = dynamic_cast<GraphContextNode*>(m_impl->graph->findNode("Globals"));
        m_impl->outputNode = dynamic_cast<CanvasOutputNode*>(m_impl->graph->findNode("Output"));
        if(!m_impl->globalsNode || !m_impl->outputNode)
            qWarning() << "CanvasClip: could not relink Globals/Output after load";
    }
}

void CanvasClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);

    m_impl->graph->drainCommandQueue();
    QJsonObject graphObj;
    m_impl->graph->writeToJson(graphObj);
    t_json.insert("graph", graphObj);
}


ClipInformation CanvasClip::info()
{
    ClipInformation toReturn([](){return new CanvasClip;});
    toReturn.name = "CanvasClip";
    toReturn.id = "canvasclip";
    //toReturn.categories.append("Generator");

    return toReturn;
}

} // namespace photon
