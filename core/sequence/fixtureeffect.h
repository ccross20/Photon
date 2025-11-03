#ifndef PHOTON_FIXTURECLIPEFFECT_H
#define PHOTON_FIXTURECLIPEFFECT_H
#include "baseeffect.h"

namespace photon {



class PHOTONCORE_EXPORT FixtureEffectEvaluationContext : public BaseEffectEvaluationContext
{
public:
    FixtureEffectEvaluationContext(ProcessContext &context):BaseEffectEvaluationContext(context){}

    Fixture *fixture = nullptr;
    int fixtureIndex = 0;
    int fixtureTotal = 0;
};

class PHOTONCORE_EXPORT FixtureEffect : public BaseEffect
{
public:
    FixtureEffect(const QByteArray &t_id = QByteArray());
    virtual ~FixtureEffect();

    FixtureClip *fixtureClip() const;
    virtual void processChannels(ProcessContext &) override;
    virtual void evaluate(FixtureEffectEvaluationContext &) = 0;
};

} // namespace photon

#endif // PHOTON_FIXTURECLIPEFFECT_H
