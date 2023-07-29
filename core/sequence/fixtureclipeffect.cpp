#include "fixtureclipeffect.h"
#include "sequence.h"
#include "clip.h"

namespace photon {

FixtureClipEffect::FixtureClipEffect(const QByteArray &t_id) : ClipEffect(t_id)
{

}

FixtureClipEffect::~FixtureClipEffect()
{

}

void FixtureClipEffect::processChannels(ProcessContext &t_context) const
{
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    FixtureClipEffectEvaluationContext localContext(t_context);
    localContext.relativeTime = initialRelativeTime;
    localContext.fixtureTotal = clip()->maskedFixtures().length();
    int index = 0;
    for(auto fixture : clip()->maskedFixtures())
    {
        if(!fixture)
            continue;
        localContext.fixtureIndex = index++;

        localContext.relativeTime = initialRelativeTime - clip()->falloff(fixture);
        if(localContext.relativeTime < 0)
            continue;

        localContext.fixture = fixture;

        prepareContext(localContext);

        evaluate(localContext);
    }
}

} // namespace photon
