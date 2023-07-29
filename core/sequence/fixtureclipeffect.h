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

    virtual void processChannels(ProcessContext &) const override;
    virtual void evaluate(FixtureClipEffectEvaluationContext &) const = 0;
};

} // namespace photon

#endif // PHOTON_FIXTURECLIPEFFECT_H
