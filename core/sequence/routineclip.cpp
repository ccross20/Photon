#include "routineclip.h"
#include "routine/routine.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "routine/routinecollection.h"
#include "project/project.h"
#include "sequence.h"
#include "model/graphinputnode.h"
#include "sequence/channel.h"

namespace photon {

class RoutineClip::Impl
{
public:
    ~Impl();
    Routine *routine = nullptr;
    RoutineClip *facade;
};

RoutineClip::Impl::~Impl()
{
    // The routine is shared — owned by the project's RoutineCollection and
    // referenced by multiple clips ("edit one updates all"). The clip must not
    // delete it.
}

RoutineClip::RoutineClip(QObject *t_parent) : FixtureClip(t_parent),m_impl(new Impl)
{
    m_impl->facade = this;
    setType("routine");
}

RoutineClip::RoutineClip(double t_start, double t_duration, QObject *t_parent) : FixtureClip(t_start, t_duration, t_parent),m_impl(new Impl)
{
    m_impl->facade = this;
    setType("routine");
}

RoutineClip::~RoutineClip()
{
    delete m_impl;
}

void RoutineClip::processChannels(ProcessContext &t_context)
{
    if(!m_impl->routine)
        return;

    double initialRelativeTime = t_context.globalTime - startTime();

    // Drive the routine's exposed inputs from this clip's channels — the timeline
    // automation. Values are constant across fixtures, so set them once before the
    // fixture loop. Bound by portId (== the channel's uniqueId, which is the input
    // node's id), not by index.
    const auto clipChannels = channels();
    for(auto *port : m_impl->routine->inputPorts())
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
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.project = t_context.project;
    localContext.strength = strengthAtTime(initialRelativeTime);

    // Model (b): evaluate the graph once. A FixtureStateNode inside iterates its
    // own fixture list and applies per-fixture offset — there is no clip-side
    // fixture loop. The clip's strength/ease envelope flows in via context.strength.
    m_impl->routine->evaluate(&localContext);
    m_impl->routine->markClean();
}

void RoutineClip::setRoutine(Routine *t_routine)
{
    if(m_impl->routine == t_routine)
        return;

    clearChannels();

    // Channel 0 is always the clip's strength envelope (the base Clip constructor
    // creates it; clearChannels() above removed it). strengthAtTime() reads
    // channel 0, so recreate it before the routine's channels.
    addChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Strength", "Strength", 1.0));

    m_impl->routine = t_routine;

    for(const auto &info : m_impl->routine->channelInfo())
    {
        addChannel(info);
    }

    setName(t_routine->name());

    connect(t_routine, &Routine::channelAdded, this, &RoutineClip::channelAddedSlot);
    connect(t_routine, &Routine::channelRemoved, this, &RoutineClip::channelRemovedSlot);
    connect(t_routine, &Routine::channelUpdated, this, &RoutineClip::routineChannelUpdatedSlot);
    emit routineChanged(m_impl->routine);
    markChanged();
}

void RoutineClip::routineChannelUpdatedSlot(int t_index)
{
    // Clip channel 0 is the strength envelope, so routine channel i lives at i+1.
    channels()[t_index + 1]->updateInfo(m_impl->routine->channelInfoAtIndex(t_index));
}

void RoutineClip::channelAddedSlot(int t_index)
{
    // Appends after the strength channel and any existing routine channels.
    addChannel(m_impl->routine->channelInfoAtIndex(t_index), t_index);
}

void RoutineClip::channelRemovedSlot(int t_index)
{
    removeChannel(t_index + 1);
}

Routine *RoutineClip::routine() const
{
    return m_impl->routine;
}

Routine *RoutineClip::contentGraph() const
{
    return m_impl->routine;
}

void RoutineClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void RoutineClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);
    m_impl->routine = t_context.project->routines()->findByUniqueId(t_json.value("routine").toString().toLatin1());
}

void RoutineClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);
    t_json.insert("routine", QString(m_impl->routine->uniqueId()));
}


} // namespace photon
