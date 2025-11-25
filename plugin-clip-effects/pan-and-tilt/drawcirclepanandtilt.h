#ifndef DRAWCIRCLEPANANDTILT_H
#define DRAWCIRCLEPANANDTILT_H
#include "sequence/fixtureeffect.h"

namespace photon {

class DrawCirclePanAndTilt : public FixtureEffect
{
public:
    DrawCirclePanAndTilt();

    void init() override;
    void evaluate(FixtureEffectEvaluationContext &) override;

    static ClipEffectInformation info();
};

} // namespace photon

#endif // DRAWCIRCLEPANANDTILT_H
