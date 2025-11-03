#ifndef PHOTON_BEATSTROBEEFFECT_H
#define PHOTON_BEATSTROBEEFFECT_H

#include "sequence/fixtureeffect.h"

namespace photon {

class BeatStrobeEffect : public FixtureEffect
{
public:
    BeatStrobeEffect();
    ~BeatStrobeEffect();

    void init() override;
    void evaluate(FixtureEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BEATSTROBEEFFECT_H
