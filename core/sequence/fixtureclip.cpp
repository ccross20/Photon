#include "fixtureclip.h"
#include "state/stateevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "project/project.h"
#include "sequence.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "routine/routine.h"
#include "routine/routineevaluationcontext.h"
#include "graph/node/fixture/fixturestatenode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/graphinputnode.h"
#include "model/parameter/parameter.h"
#include "graph/parameter/colorparameter.h"
#include "graph/parameter/point2dparameter.h"
#include "sequence/channel.h"
#include "state/statecapability.h"

namespace photon {


class FixtureClip::Impl
{
public:
    Impl(FixtureClip *t_facade);
    ~Impl();
    void createGraph();
    void connectGraphSignals();
    // The clip's content graph — an inline (privately owned) routine whose default
    // content is a single FixtureStateNode. The node holds the State, iterates the
    // fixtures its Fixtures port resolves to, and applies per-fixture offset.
    // Model (b): the clip evaluates this graph once; the node does the fixture loop.
    Routine *graph = nullptr;
    FixtureStateNode *stateNode = nullptr;
    FixtureClip *facade;
};

FixtureClip::Impl::Impl(FixtureClip *t_facade)
{
    facade = t_facade;
}

FixtureClip::Impl::~Impl()
{
    if(graph)
        delete graph;   // owns stateNode
}

void FixtureClip::Impl::createGraph()
{
    graph = new Routine;
    graph->setGraphTypeId("routine");
    stateNode = new FixtureStateNode;
    stateNode->createParameters();
    graph->addNode(stateNode);
    graph->drainCommandQueue();
    connectGraphSignals();
}

void FixtureClip::Impl::connectGraphSignals()
{
    // Mirror the graph's input ports as clip channels, so any input node — exposed
    // via the Channel button or added by hand in the graph — appears in the editor.
    QObject::connect(graph, &keira::Graph::interfaceChanged, facade,
                     [f = facade]() { f->syncChannelsFromGraph(); });
}

FixtureClip::FixtureClip(QObject *t_parent) : Clip(t_parent),m_impl(new Impl(this))
{
    m_impl->createGraph();
    setType("fixture");
    setId("fixtureclip");
}

FixtureClip::FixtureClip(double t_start, double t_duration, QObject *t_parent) : Clip(t_start, t_duration, t_parent),m_impl(new Impl(this))
{
    m_impl->createGraph();
    setType("fixture");
    setId("fixtureclip");
}

FixtureClip::~FixtureClip()
{
    delete m_impl;
}

bool FixtureClip::timeIsValid(double t_time) const
{
    return startTime() < t_time && t_time < endTime();
}

void FixtureClip::processChannels(ProcessContext &t_context)
{
    if(!m_impl->graph || !m_impl->stateNode)
        return;

    // Apply any queued graph edits (e.g. a capability just exposed from the editor)
    // at this safe point before evaluating.
    m_impl->graph->drainCommandQueue();

    double initialRelativeTime = t_context.globalTime - startTime();

    // The clip no longer seeds the fixture selection - fixture selection and
    // per-fixture offset are the graph's job now. FixtureStateNode's Fixtures
    // port resolves to every fixture in the project while nothing is wired
    // into it, so an untouched clip still covers the whole rig; wiring a
    // SelectFixturesNode / falloff node into that port narrows or offsets it.

    // Drive exposed graph inputs from this clip's channels (timeline automation),
    // bound by portId == channel uniqueId.
    const auto clipChannels = channels();
    for(auto *port : m_impl->graph->inputPorts())
    {
        for(auto *channel : clipChannels)
        {
            if(channel->uniqueId() == port->portId())
            {
                port->setPortValue(channel->processValue(initialRelativeTime));
                break;
            }
        }
    }

    RoutineEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime   = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.project      = t_context.project;
    localContext.strength     = strengthAtTime(initialRelativeTime);

    // Model (b): evaluate the graph once — the FixtureStateNode does the fixtures.
    m_impl->graph->evaluate(&localContext);
    m_impl->graph->markClean();
}

State *FixtureClip::state() const
{
    return m_impl->stateNode ? m_impl->stateNode->state() : nullptr;
}

Routine *FixtureClip::contentGraph() const
{
    return m_impl->graph;
}

void FixtureClip::syncChannelsFromGraph()
{
    if(!m_impl->graph)
        return;

    // Add a channel for every input port that doesn't have one yet.
    QSet<QByteArray> portIds;
    for(auto *port : m_impl->graph->inputPorts())
    {
        portIds.insert(port->portId());

        // If a channel already exists for this port, keep it — but refresh its name
        // so renaming the input node renames the channel.
        Channel *existing = nullptr;
        for(auto *channel : channels())
            if(channel->uniqueId() == port->portId()) { existing = channel; break; }
        if(existing)
        {
            if(existing->info().name != port->portName())
            {
                ChannelInfo info = existing->info();
                info.name = port->portName();
                existing->updateInfo(info);
            }
            continue;
        }

        ChannelInfo info;
        info.name     = port->portName();
        info.uniqueId = port->portId();
        auto *valuePort = port->valuePort();
        const QByteArray typeId = valuePort ? valuePort->typeId() : QByteArray();
        if(typeId == ColorParameter::ParameterId)
            info.type = ChannelInfo::ChannelTypeColor;
        else if(typeId == Point2DParameter::ParameterId)
            info.type = ChannelInfo::ChannelTypePoint;
        else
            info.type = ChannelInfo::ChannelTypeNumber;
        if(valuePort)
            info.defaultValue = valuePort->value();
        addChannel(info);
    }

    // Remove graph-derived channels whose port is gone. Channel 0 is the strength
    // envelope and is always kept.
    for(int i = channelCount() - 1; i >= 1; --i)
        if(!portIds.contains(channelAtIndex(i)->uniqueId()))
            removeChannel(i);
}

void FixtureClip::exposeCapabilityChannel(StateCapability *t_cap, int t_index)
{
    if(!m_impl->graph || !m_impl->stateNode || !t_cap)
        return;

    const auto capChannels = t_cap->availableChannels();
    if(t_index < 0 || t_index >= capChannels.size())
        return;

    const ChannelInfo capInfo = capChannels.at(t_index);
    const QByteArray channelId = t_cap->channelId(t_index);

    // Pick the input node type that matches the capability channel.
    QByteArray inputNodeId;
    switch(capInfo.type)
    {
    case ChannelInfo::ChannelTypeColor: inputNodeId = "photon.routine.color-input"; break;
    case ChannelInfo::ChannelTypePoint: inputNodeId = "photon.routine.point-input"; break;
    default:                            inputNodeId = "photon.routine.number-input"; break;
    }

    auto *inputNode = dynamic_cast<keira::GraphInputNode*>(
        photonApp->plugins()->nodeLibrary()->createNode(inputNodeId));
    if(!inputNode)
        return;
    inputNode->setName(capInfo.name);
    inputNode->setValue("name", capInfo.name);   // the exposed-input name (portName)
    inputNode->setPosition(m_impl->stateNode->position() - QPointF(280, 0));

    // Queue the graph edits; they drain in order (expose port, add node, connect)
    // at the start of the next processChannels(). We connect by id because the
    // exposed port does not exist yet at this point.
    m_impl->stateNode->setChannelExposed(t_cap, t_index, true);
    m_impl->graph->addNode(inputNode);
    if(inputNode->valuePort())
        m_impl->graph->connectParameters(
            inputNode->uniqueId() + "/" + inputNode->valuePort()->id(),
            m_impl->stateNode->uniqueId() + "/" + channelId);

    // Bind a clip channel to the input node so processChannels() drives it. The
    // channel carries the capability's type/name but its id matches the input
    // node's portId (the binding key).
    ChannelInfo channelInfo = capInfo;
    channelInfo.uniqueId = inputNode->portId();
    addChannel(channelInfo);
}

void FixtureClip::unexposeCapabilityChannel(StateCapability *t_cap, int t_index)
{
    if(!m_impl->graph || !m_impl->stateNode || !t_cap)
        return;

    const QByteArray channelId = t_cap->channelId(t_index);

    // Remove the input node feeding the exposed port. Its removal fires
    // interfaceChanged → syncChannelsFromGraph drops the bound clip channel.
    if(auto *port = m_impl->stateNode->findParameter(channelId))
        if(auto *source = port->inputParameter())
            if(auto *node = source->node())
                m_impl->graph->removeNode(node);

    // Drop the exposed capability port (also disconnects it).
    m_impl->stateNode->setChannelExposed(t_cap, t_index, false);
}

bool FixtureClip::isCapabilityChannelExposed(StateCapability *t_cap, int t_index) const
{
    if(!m_impl->stateNode || !t_cap)
        return false;
    return m_impl->stateNode->isChannelExposed(t_cap, t_index);
}

void FixtureClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void FixtureClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);

    // Older files may still carry "defaultFalloff"/"falloffEffects"/"maskEffects"
    // keys from when selection masks and falloff lived on the clip itself; they're
    // silently ignored now that the clip's graph owns both jobs.

    if(t_json.contains("graph"))
    {
        delete m_impl->graph;
        m_impl->graph = new Routine;
        m_impl->graph->setGraphTypeId("routine");
        m_impl->graph->readFromJson(t_json.value("graph").toObject(), photonApp->plugins()->nodeLibrary());
        m_impl->stateNode = nullptr;
        for(auto *node : m_impl->graph->nodes())
            if(auto *sn = dynamic_cast<FixtureStateNode*>(node)) { m_impl->stateNode = sn; break; }
        m_impl->connectGraphSignals();
    }
    else if(t_json.contains("state") && m_impl->stateNode)
    {
        // Legacy clips stored the State directly on the clip — load it into the node.
        auto stateObj = t_json.value("state").toObject();
        m_impl->stateNode->state()->readFromJson(stateObj, t_context);
    }
}

void FixtureClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);

    if(m_impl->graph)
    {
        // Exposing/unexposing a capability channel (editor action) queues its
        // node/connection edits for the next processChannels() - which only runs
        // once the playhead is actually over this clip. If the clip never gets
        // evaluated between that edit and a save, the edit is still sitting
        // unapplied here and would otherwise be silently dropped from the file
        // (the channel itself is added immediately, so it'd survive alone,
        // orphaned with nothing driving it). Flush before serializing so the
        // save always reflects the latest intent.
        m_impl->graph->drainCommandQueue();
        QJsonObject graphObj;
        m_impl->graph->writeToJson(graphObj);
        t_json.insert("graph", graphObj);
    }
}


ClipInformation FixtureClip::info()
{
    ClipInformation toReturn([](){return new FixtureClip;});
    toReturn.name = "FixtureClip";
    toReturn.id = "fixtureclip";
    //toReturn.categories.append("Generator");

    return toReturn;
}


} // namespace photon
