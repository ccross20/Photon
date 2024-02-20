#include "fixtureclipeffect.h"
#include "sequence.h"
#include "fixtureclip.h"

namespace photon {

FixtureClipEffect::FixtureClipEffect(const QByteArray &t_id) : ClipEffect(t_id)
{

}

FixtureClipEffect::~FixtureClipEffect()
{

}

FixtureClip *FixtureClipEffect::fixtureClip() const
{
    return static_cast<FixtureClip*>(clip());
}

void FixtureClipEffect::processChannels(ProcessContext &t_context)
{
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    FixtureClipEffectEvaluationContext localContext(t_context);
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
