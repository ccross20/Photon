#include "stateclip.h"
#include "state/stateevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "project/project.h"
#include "fixture/maskeffect.h"
#include "sequence.h"
#include "fixture/fixture.h"

namespace photon {

class StateClip::Impl
{
public:
    ~Impl();
    void processFixture(Fixture *, StateEvaluationContext &, double);
    State *state = nullptr;
    StateClip *facade;
};

StateClip::Impl::~Impl()
{
    if(state)
        delete state;
}

void StateClip::Impl::processFixture(Fixture *t_fixture, StateEvaluationContext &t_context, double initialRelativeTime)
{
    t_context.relativeTime = initialRelativeTime - facade->falloff(t_fixture);

    if(t_context.relativeTime < 0)
        return;

    t_context.fixture = t_fixture;
    t_context.strength = facade->strengthAtTime(t_context.relativeTime);

    t_context.channelValues.clear();
    state->initializeValues(t_context);

    for(const auto &channel : facade->channels())
    {
        if(channel->type() == ChannelInfo::ChannelTypeColor)
            t_context.channelValues.insert(channel->uniqueId(), channel->processColor(t_context.relativeTime));
        else
            t_context.channelValues.insert(channel->uniqueId(), channel->processDouble(t_context.relativeTime));

    }

    //t_context.strength = facade->channelAtIndex(0)->processDouble(t_context.relativeTime);

    state->evaluate(t_context);
}

StateClip::StateClip() : Clip(),m_impl(new Impl)
{
    m_impl->facade = this;
    setType("state");
}

StateClip::StateClip(double t_start, double t_duration, QObject *t_parent) : Clip(t_start, t_duration, t_parent),m_impl(new Impl)
{
    m_impl->facade = this;
    setType("state");
}

StateClip::~StateClip()
{
    delete m_impl;
}

QString StateClip::name() const
{
    return "State";
}

void StateClip::processChannels(ProcessContext &t_context)
{
    if(!m_impl->state)
        return;

    double initialRelativeTime = t_context.globalTime - startTime();

    StateEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.fixture = t_context.fixture;

    for(auto fixture : maskedFixtures())
    {
        m_impl->processFixture(fixture, localContext, initialRelativeTime);
    }

}

void StateClip::setState(State *t_state)
{
    m_impl->state = t_state;
}

State *StateClip::state() const
{
    return m_impl->state;
}

void StateClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void StateClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);

    m_impl->state = new State;

    if(t_json.contains("state"))
    {
        auto stateObj = t_json.value("state").toObject();
        m_impl->state->readFromJson(stateObj, t_context);
    }

}

void StateClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);

    if(m_impl->state)
    {
        QJsonObject stateObj;
        m_impl->state->writeToJson(stateObj);
        t_json.insert("state", stateObj);
    }
}


} // namespace photon
