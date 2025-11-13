#ifndef COLORSPARKLE_H
#define COLORSPARKLE_H
#include <QEasingCurve>
#include <QRandomGenerator>
#include "sequence/fixtureeffect.h"

namespace photon {

class ColorSparkle : public FixtureEffect
{
public:
    ColorSparkle();

    void init() override;
    void evaluate(FixtureEffectEvaluationContext &) override;

    static ClipEffectInformation info();

private:
    QVector<float> m_offsets;
    QEasingCurve m_easingIn;
    QEasingCurve m_easingOut;
    QRandomGenerator m_random;
};

} // namespace photon

#endif // COLORSPARKLE_H
