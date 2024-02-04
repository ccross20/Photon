#ifndef PHOTON_BEATSTROBEEFFECT_H
#define PHOTON_BEATSTROBEEFFECT_H

#include "sequence/fixtureclipeffect.h"

namespace photon {

class BeatStrobeEffect : public FixtureClipEffect
{
public:
    BeatStrobeEffect();
    ~BeatStrobeEffect();

    void init() override;
    void evaluate(FixtureClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BEATSTROBEEFFECT_H
