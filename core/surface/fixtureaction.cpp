#include "fixtureaction.h"
#include "state/stateevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "project/project.h"
#include "fixture/maskeffect.h"
#include "fixture/fixture.h"
#include "falloff/falloffeffect_p.h"
#include "falloff/constantfalloffeffect.h"
#include "fixture/maskeffect_p.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "state/state.h"

namespace photon {


class FixtureAction::Impl
{
public:
    Impl(FixtureAction *t_facade);
    ~Impl();
    double falloff(Fixture *);
    void processFixture(Fixture *, StateEvaluationContext &, double);
    State *state = nullptr;
    QVector<MaskEffect*> maskEffects;
    QVector<FalloffEffect*> falloffEffects;
    FixtureAction *facade;
    double defaultFalloff = 0;
};

FixtureAction::Impl::Impl(FixtureAction *t_facade)
{
    falloffEffects.append(photonApp->plugins()->createFalloffEffect(ConstantFalloffEffect::info().effectId));
    falloffEffects.back()->m_impl->parent = t_facade;
    facade = t_facade;

}

FixtureAction::Impl::~Impl()
{
    if(state)
        delete state;
}

double FixtureAction::Impl::falloff(Fixture *t_fixture)
{
    if(falloffEffects.isEmpty())
        return 0.0;

    return falloffEffects.back()->falloff(t_fixture);
}

void FixtureAction::Impl::processFixture(Fixture *t_fixture, StateEvaluationContext &t_context, double initialRelativeTime)
{
    //t_context.relativeTime = initialRelativeTime - facade->falloff(t_fixture);

    t_context.fixture = t_fixture;
    t_context.strength = facade->strengthAtTime(t_context.relativeTime);

    t_context.channelValues.clear();
    state->initializeValues(t_context);

    for(const auto &channel : facade->channels())
    {
        t_context.channelValues.insert(channel->uniqueId(), channel->processValue(t_context.relativeTime));
    }

    //t_context.strength = facade->channelAtIndex(0)->processDouble(t_context.relativeTime);

    state->evaluate(t_context);
}

FixtureAction::FixtureAction(QObject *t_parent) : SurfaceAction(t_parent),m_impl(new Impl(this))
{
    m_impl->state = new State;
    m_impl->state->setParent(this);
    setType("fixture");
    setId("fixtureaction");
}

FixtureAction::~FixtureAction()
{
    for(auto effect : m_impl->falloffEffects)
        delete effect;
    delete m_impl;
}

double FixtureAction::falloff(Fixture *t_fixture) const
{
    return m_impl->falloff(t_fixture);
}

void FixtureAction::setDefaultFalloff(double t_falloff)
{
    m_impl->defaultFalloff = t_falloff;
    markChanged();
    emit actionUpdated(this);
}

double FixtureAction::defaultFalloff() const
{
    return m_impl->defaultFalloff;
}

void FixtureAction::addFalloffEffect(FalloffEffect *t_effect)
{
    if(!m_impl->falloffEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->falloffEffects.back();
    m_impl->falloffEffects.append(t_effect);
    t_effect->m_impl->parent = this;

    emit falloffEffectAdded(t_effect);
    markChanged();
}

void FixtureAction::removeFalloffEffect(FalloffEffect *t_effect)
{
    if(m_impl->falloffEffects.removeOne(t_effect))
    {
        t_effect->m_impl->parent = nullptr;
        for(uint i = 1; i < m_impl->falloffEffects.length(); ++i)
        {
            m_impl->falloffEffects[i]->m_impl->previousEffect = m_impl->falloffEffects[i-1];
        }

        emit falloffEffectRemoved(t_effect);
    }
}

FalloffEffect *FixtureAction::falloffEffectAtIndex(int t_index) const
{
    return m_impl->falloffEffects[t_index];
}

void FixtureAction::falloffUpdatedSlot(photon::FalloffEffect *t_falloff)
{
    emit falloffUpdated(t_falloff);
    markChanged();
}

void FixtureAction::addMaskEffect(MaskEffect *t_effect)
{
    if(!m_impl->maskEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->maskEffects.back();
    m_impl->maskEffects.append(t_effect);
    t_effect->m_impl->parent = this;

    emit maskAdded(t_effect);
    markChanged();
}

void FixtureAction::removeMaskEffect(MaskEffect *t_effect)
{
    if(m_impl->maskEffects.removeOne(t_effect))
    {
        t_effect->m_impl->parent = nullptr;
        for(uint i = 1; i < m_impl->falloffEffects.length(); ++i)
        {
            m_impl->maskEffects[i]->m_impl->previousEffect = m_impl->maskEffects[i-1];
        }

        emit maskRemoved(t_effect);
    }
}

MaskEffect *FixtureAction::maskEffectAtIndex(int index) const
{
    return m_impl->maskEffects[index];
}

int FixtureAction::maskEffectCount() const
{
    return m_impl->maskEffects.length();
}

const QVector<Fixture*> FixtureAction::maskedFixtures() const
{
    if(m_impl->maskEffects.isEmpty())
        return photonApp->project()->fixtures()->fixtures();
    return m_impl->maskEffects.back()->process(photonApp->project()->fixtures()->fixtures());
}

void FixtureAction::maskUpdatedSlot(photon::MaskEffect *t_mask)
{
    emit maskUpdated(t_mask);
    markChanged();
}

int FixtureAction::falloffEffectCount() const
{
    return m_impl->falloffEffects.length();
}

void FixtureAction::processChannels(ProcessContext &t_context)
{
    if(!m_impl->state)
        return;


    if(strengthAtTime(t_context.relativeTime) <= 0)
        return;

    double initialRelativeTime = t_context.globalTime - startTime();

    StateEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = t_context.globalTime;
    localContext.fixture = t_context.fixture;

    for(auto fixture : maskedFixtures())
    {
        m_impl->processFixture(fixture, localContext, initialRelativeTime);
    }

    SurfaceAction::processChannels(t_context);

}

State *FixtureAction::state() const
{
    return m_impl->state;
}

void FixtureAction::restore(Project &t_project)
{
    SurfaceAction::restore(t_project);
}

void FixtureAction::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceAction::readFromJson(t_json, t_context);

    for(auto effect : m_impl->falloffEffects)
        delete effect;
    m_impl->falloffEffects.clear();
    m_impl->defaultFalloff = t_json.value("defaultFalloff").toDouble();

    if(m_impl->state)
        delete m_impl->state;
    m_impl->state = new State;
    m_impl->state->setParent(this);

    if(t_json.contains("state"))
    {
        auto stateObj = t_json.value("state").toObject();
        m_impl->state->readFromJson(stateObj, t_context);
    }


    if(t_json.contains("falloffEffects"))
    {
        auto effectArray = t_json.value("falloffEffects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createFalloffEffect(id.toLatin1());

            if(effect){
                effect->readFromJson(effectObj);
                if(!m_impl->falloffEffects.isEmpty())
                    effect->m_impl->previousEffect = m_impl->falloffEffects.back();
                m_impl->falloffEffects.append(effect);

                effect->m_impl->parent = this;
            }
        }
    }

    if(t_json.contains("maskEffects"))
    {
        auto effectArray = t_json.value("maskEffects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createMaskEffect(id.toLatin1());

            if(effect){
                effect->readFromJson(effectObj);
                if(!m_impl->maskEffects.isEmpty())
                    effect->m_impl->previousEffect = m_impl->maskEffects.back();
                m_impl->maskEffects.append(effect);

                effect->m_impl->parent = this;
            }
        }
    }

}

void FixtureAction::writeToJson(QJsonObject &t_json) const
{
    SurfaceAction::writeToJson(t_json);
    t_json.insert("defaultFalloff", m_impl->defaultFalloff);

    if(m_impl->state)
    {
        QJsonObject stateObj;
        m_impl->state->writeToJson(stateObj);
        t_json.insert("state", stateObj);
    }

    QJsonArray falloffArray;
    for(auto effect : m_impl->falloffEffects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        falloffArray.append(effectObj);
    }
    t_json.insert("falloffEffects", falloffArray);


    QJsonArray maskArray;
    for(auto effect : m_impl->maskEffects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        maskArray.append(effectObj);
    }
    t_json.insert("maskEffects", maskArray);
}


} // namespace photon
