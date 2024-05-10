#include "fixtureclip.h"
#include "state/stateevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "project/project.h"
#include "fixture/maskeffect.h"
#include "sequence.h"
#include "fixture/fixture.h"
#include "falloff/falloffeffect_p.h"
#include "falloff/constantfalloffeffect.h"
#include "fixture/maskeffect_p.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {


class FixtureClip::Impl
{
public:
    Impl();
    ~Impl();
    double falloff(Fixture *);
    void processFixture(Fixture *, StateEvaluationContext &, double);
    State *state = nullptr;
    QVector<MaskEffect*> maskEffects;
    QVector<FalloffEffect*> falloffEffects;
    FixtureClip *facade;
    double defaultFalloff = 0;
};

FixtureClip::Impl::Impl()
{
    falloffEffects.append(photonApp->plugins()->createFalloffEffect(ConstantFalloffEffect::info().effectId));
    falloffEffects.back()->m_impl->clip = facade;

}

FixtureClip::Impl::~Impl()
{
    if(state)
        delete state;
}

double FixtureClip::Impl::falloff(Fixture *t_fixture)
{
    if(falloffEffects.isEmpty())
        return 0.0;

    return falloffEffects.back()->falloff(t_fixture);
}

void FixtureClip::Impl::processFixture(Fixture *t_fixture, StateEvaluationContext &t_context, double initialRelativeTime)
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
        t_context.channelValues.insert(channel->uniqueId(), channel->processValue(t_context.relativeTime));
    }

    //t_context.strength = facade->channelAtIndex(0)->processDouble(t_context.relativeTime);

    state->evaluate(t_context);
}

FixtureClip::FixtureClip(QObject *t_parent) : Clip(t_parent),m_impl(new Impl)
{
    m_impl->facade = this;
    m_impl->state = new State;
    setType("fixture");
    setId("fixtureclip");
}

FixtureClip::FixtureClip(double t_start, double t_duration, QObject *t_parent) : Clip(t_start, t_duration, t_parent),m_impl(new Impl)
{
    m_impl->facade = this;
    m_impl->state = new State;
    setType("fixture");
    setId("fixtureclip");
}

FixtureClip::~FixtureClip()
{
    for(auto effect : m_impl->falloffEffects)
        delete effect;
    delete m_impl;
}

bool FixtureClip::timeIsValid(double t_time) const
{
    return startTime() < t_time && t_time < endTime() + m_impl->defaultFalloff;
}

double FixtureClip::falloff(Fixture *t_fixture) const
{
    return m_impl->falloff(t_fixture);
}

void FixtureClip::setDefaultFalloff(double t_falloff)
{
    m_impl->defaultFalloff = t_falloff;
    markChanged();
    emit clipUpdated(this);
}

double FixtureClip::defaultFalloff() const
{
    return m_impl->defaultFalloff;
}

void FixtureClip::addFalloffEffect(FalloffEffect *t_effect)
{
    if(!m_impl->falloffEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->falloffEffects.back();
    m_impl->falloffEffects.append(t_effect);
    t_effect->m_impl->clip = this;

    emit falloffEffectAdded(t_effect);
    markChanged();
}

void FixtureClip::removeFalloffEffect(FalloffEffect *t_effect)
{
    if(m_impl->falloffEffects.removeOne(t_effect))
    {
        t_effect->m_impl->clip = nullptr;
        for(uint i = 1; i < m_impl->falloffEffects.length(); ++i)
        {
            m_impl->falloffEffects[i]->m_impl->previousEffect = m_impl->falloffEffects[i-1];
        }

        emit falloffEffectRemoved(t_effect);
    }
}

FalloffEffect *FixtureClip::falloffEffectAtIndex(int t_index) const
{
    return m_impl->falloffEffects[t_index];
}

void FixtureClip::falloffUpdatedSlot(photon::FalloffEffect *t_falloff)
{
    emit falloffUpdated(t_falloff);
    markChanged();
}

void FixtureClip::addMaskEffect(MaskEffect *t_effect)
{
    if(!m_impl->maskEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->maskEffects.back();
    m_impl->maskEffects.append(t_effect);
    t_effect->m_impl->clip = this;

    emit maskAdded(t_effect);
    markChanged();
}

void FixtureClip::removeMaskEffect(MaskEffect *t_effect)
{
    if(m_impl->maskEffects.removeOne(t_effect))
    {
        t_effect->m_impl->clip = nullptr;
        for(uint i = 1; i < m_impl->falloffEffects.length(); ++i)
        {
            m_impl->maskEffects[i]->m_impl->previousEffect = m_impl->maskEffects[i-1];
        }

        emit maskRemoved(t_effect);
    }
}

MaskEffect *FixtureClip::maskEffectAtIndex(int index) const
{
    return m_impl->maskEffects[index];
}

int FixtureClip::maskEffectCount() const
{
    return m_impl->maskEffects.length();
}

const QVector<Fixture*> FixtureClip::maskedFixtures() const
{
    if(m_impl->maskEffects.isEmpty())
        return photonApp->project()->fixtures()->fixtures();
    return m_impl->maskEffects.back()->process(photonApp->project()->fixtures()->fixtures());
}

void FixtureClip::maskUpdatedSlot(photon::MaskEffect *t_mask)
{
    emit maskUpdated(t_mask);
    markChanged();
}

int FixtureClip::falloffEffectCount() const
{
    return m_impl->falloffEffects.length();
}

void FixtureClip::processChannels(ProcessContext &t_context)
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

    Clip::processChannels(t_context);

}

State *FixtureClip::state() const
{
    return m_impl->state;
}

void FixtureClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void FixtureClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);

    for(auto effect : m_impl->falloffEffects)
        delete effect;
    m_impl->falloffEffects.clear();
    m_impl->defaultFalloff = t_json.value("defaultFalloff").toDouble();

    m_impl->state = new State;

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

                effect->m_impl->clip = this;
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

                effect->m_impl->clip = this;
            }
        }
    }

}

void FixtureClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);
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


ClipInformation FixtureClip::info()
{
    ClipInformation toReturn([](){return new FixtureClip;});
    toReturn.name = "FixtureClip";
    toReturn.id = "fixtureclip";
    //toReturn.categories.append("Generator");

    return toReturn;
}


} // namespace photon
