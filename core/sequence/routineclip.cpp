#include "routineclip.h"
#include "routine/routine.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "routine/routinecollection.h"
#include "project/project.h"
#include "sequence.h"

namespace photon {

class RoutineClip::Impl
{
public:
    ~Impl();
    void processFixture(Fixture *, RoutineEvaluationContext &, double);
    Routine *routine = nullptr;
    RoutineClip *facade;
};

RoutineClip::Impl::~Impl()
{
    if(routine)
        delete routine;
}

void RoutineClip::Impl::processFixture(Fixture *t_fixture, RoutineEvaluationContext &t_context, double initialRelativeTime)
{
    t_context.relativeTime = initialRelativeTime - facade->falloff(t_fixture);
    if(t_context.relativeTime < 0)
        return;

    t_context.fixture = t_fixture;
    t_context.strength = facade->strengthAtTime(t_context.relativeTime);

    t_context.channelValues.clear();
    for(const auto &channel : facade->channels())
    {
        t_context.channelValues.insert(channel->uniqueId(), channel->processDouble(t_context.relativeTime));
    }

    routine->evaluate(&t_context);
    routine->markClean();
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

    RoutineEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.fixture = t_context.fixture;
    localContext.project = t_context.project;

    for(auto fixture : maskedFixtures())
    {
        m_impl->processFixture(fixture, localContext, initialRelativeTime);
    }
}

void RoutineClip::setRoutine(Routine *t_routine)
{
    if(m_impl->routine == t_routine)
        return;

    clearChannels();
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
    channels()[t_index]->updateInfo(m_impl->routine->channelInfoAtIndex(t_index));
}

void RoutineClip::channelAddedSlot(int t_index)
{
    addChannel(m_impl->routine->channelInfoAtIndex(t_index), t_index);
}

void RoutineClip::channelRemovedSlot(int t_index)
{
    removeChannel(t_index);
}

Routine *RoutineClip::routine() const
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
