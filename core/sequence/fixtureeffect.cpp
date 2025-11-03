#include "fixtureeffect.h"
#include "sequence.h"
#include "fixtureclip.h"

namespace photon {

FixtureEffect::FixtureEffect(const QByteArray &t_id) : BaseEffect(t_id)
{

}

FixtureEffect::~FixtureEffect()
{

}

FixtureClip *FixtureEffect::fixtureClip() const
{
    return static_cast<FixtureClip*>(effectParent());
}

void FixtureEffect::processChannels(ProcessContext &t_context)
{
    double initialRelativeTime = t_context.globalTime - effectParent()->startTime();

    FixtureEffectEvaluationContext localContext(t_context);
    localContext.relativeTime = initialRelativeTime;
    localContext.fixtureTotal = fixtureClip()->maskedFixtures().length();
    int index = 0;
    for(auto fixture : fixtureClip()->maskedFixtures())
    {
        if(!fixture)
            continue;
        localContext.fixtureIndex = index++;

        localContext.relativeTime = initialRelativeTime - fixtureClip()->falloff(fixture);
        if(localContext.relativeTime < 0)
            continue;

        localContext.fixture = fixture;

        prepareContext(localContext);

        evaluate(localContext);
    }
}

} // namespace photon
