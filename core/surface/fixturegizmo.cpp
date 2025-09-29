#include "fixturegizmo.h"
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

namespace photon {

class FixtureGizmo::Impl
{
public:
    Impl(FixtureGizmo *t_facade);
    ~Impl();
    double falloff(Fixture *);
    void processFixture(Fixture *, StateEvaluationContext &, double);
    State *state = nullptr;
    QVector<MaskEffect*> maskEffects;
    QVector<FalloffEffect*> falloffEffects;
    FixtureGizmo *facade;
    double defaultFalloff = 0;

};


FixtureGizmo::Impl::Impl(FixtureGizmo *t_facade)
{
    falloffEffects.append(photonApp->plugins()->createFalloffEffect(ConstantFalloffEffect::info().effectId));
    falloffEffects.back()->m_impl->parent = t_facade;
    facade = t_facade;

}

FixtureGizmo::Impl::~Impl()
{
    if(state)
        delete state;
}

double FixtureGizmo::Impl::falloff(Fixture *t_fixture)
{
    if(falloffEffects.isEmpty())
        return 0.0;

    return falloffEffects.back()->falloff(t_fixture);
}

void FixtureGizmo::Impl::processFixture(Fixture *t_fixture, StateEvaluationContext &t_context, double initialRelativeTime)
{
    t_context.relativeTime = initialRelativeTime - facade->falloff(t_fixture);

    if(t_context.relativeTime < 0)
        return;

    t_context.fixture = t_fixture;
    //t_context.strength = facade->strengthAtTime(t_context.relativeTime);

    t_context.channelValues.clear();
    state->initializeValues(t_context);

    /*
    for(const auto &channel : facade->channels())
    {
        t_context.channelValues.insert(channel->uniqueId(), channel->processValue(t_context.relativeTime));
    }
*/

    //t_context.strength = facade->channelAtIndex(0)->processDouble(t_context.relativeTime);

    state->evaluate(t_context);
}

FixtureGizmo::FixtureGizmo(QObject *t_parent) : SurfaceGizmo("Fixture",t_parent),m_impl(new Impl(this))
{
    m_impl->state = new State;
    setType("fixture");
    setId("fixtureclip");
}


FixtureGizmo::~FixtureGizmo()
{
    delete m_impl;
}

void FixtureGizmo::addMaskEffect(MaskEffect *t_effect)
{
    if(!m_impl->maskEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->maskEffects.back();
    m_impl->maskEffects.append(t_effect);
    t_effect->m_impl->parent = this;

    emit maskAdded(t_effect);
    markChanged();
}

void FixtureGizmo::removeMaskEffect(MaskEffect *t_effect)
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

MaskEffect *FixtureGizmo::maskEffectAtIndex(int index) const
{
    return m_impl->maskEffects[index];
}

int FixtureGizmo::maskEffectCount() const
{
    return m_impl->maskEffects.length();
}

const QVector<Fixture*> FixtureGizmo::maskedFixtures() const
{
    if(m_impl->maskEffects.isEmpty())
        return photonApp->project()->fixtures()->fixtures();
    return m_impl->maskEffects.back()->process(photonApp->project()->fixtures()->fixtures());
}


void FixtureGizmo::addFalloffEffect(FalloffEffect *t_effect)
{
    if(!m_impl->falloffEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->falloffEffects.back();
    m_impl->falloffEffects.append(t_effect);
    t_effect->m_impl->parent = this;

    emit falloffEffectAdded(t_effect);
    markChanged();
}

void FixtureGizmo::removeFalloffEffect(FalloffEffect *t_effect)
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

FalloffEffect *FixtureGizmo::falloffEffectAtIndex(int t_index) const
{
    return m_impl->falloffEffects[t_index];
}

int FixtureGizmo::falloffEffectCount() const
{
    return m_impl->falloffEffects.length();
}

void FixtureGizmo::setDefaultFalloff(double)
{

}

double FixtureGizmo::defaultFalloff() const
{
    return 0.0;
}

double FixtureGizmo::falloff(Fixture *t_fixture) const
{
    return m_impl->falloff(t_fixture);
}


State *FixtureGizmo::state() const
{
    return m_impl->state;
}


void FixtureGizmo::processChannels(ProcessContext &t_context)
{
    if(!m_impl->state)
        return;

    double initialRelativeTime = 0.0;

    StateEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.fixture = t_context.fixture;

    for(auto fixture : maskedFixtures())
    {
        m_impl->processFixture(fixture, localContext, initialRelativeTime);
    }
}

void FixtureGizmo::restore(Project &)
{

}

void FixtureGizmo::readFromJson(const QJsonObject &, const LoadContext &)
{

}

void FixtureGizmo::writeToJson(QJsonObject &) const
{

}


} // namespace photo
