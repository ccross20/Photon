#ifndef PHOTON_FIXTURECLIPEFFECT_H
#define PHOTON_FIXTURECLIPEFFECT_H
#include "clipeffect.h"

namespace photon {



class PHOTONCORE_EXPORT FixtureClipEffectEvaluationContext : public ClipEffectEvaluationContext
{
public:
    FixtureClipEffectEvaluationContext(ProcessContext &context):ClipEffectEvaluationContext(context){}

    Fixture *fixture = nullptr;
    int fixtureIndex = 0;
    int fixtureTotal = 0;
};

class PHOTONCORE_EXPORT FixtureClipEffect : public ClipEffect
{
public:
    FixtureClipEffect(const QByteArray &t_id = QByteArray());
    virtual ~FixtureClipEffect();

    FixtureClip *fixtureClip() const;
    virtual void processChannels(ProcessContext &) override;
    virtual void evaluate(FixtureClipEffectEvaluationContext &) = 0;
};

} // namespace photon

#endif // PHOTON_FIXTURECLIPEFFECT_H
