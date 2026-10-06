#ifndef PHOTON_DURATIONCURVEEFFECT_H
#define PHOTON_DURATIONCURVEEFFECT_H

#include <QEasingCurve>
#include "sequence/channeleffect.h"

namespace photon {

// A rise-hold-fall shape that always fits its clip: Min at the clip's start,
// eased up to Max, held, then eased back to Min by the clip's end. The ease
// durations are fractions of the clip's length (0..1), so the shape keeps its
// proportions however the clip is resized. If the two add up to more than 1
// they're scaled down to share the clip - both at 1 gives each half.
//
// Sets the channel's value outright (Min and Max are absolute), replacing
// whatever effects before it produced.
class DurationCurveEffect : public ChannelEffect
{
public:
    DurationCurveEffect();

    double minimum() const { return m_minimum; }
    double maximum() const { return m_maximum; }
    double easeInDuration() const { return m_easeInDuration; }
    double easeOutDuration() const { return m_easeOutDuration; }
    QEasingCurve::Type easeInType() const { return m_easeInType; }
    QEasingCurve::Type easeOutType() const { return m_easeOutType; }

    void setMinimum(double);
    void setMaximum(double);
    void setEaseInDuration(double);
    void setEaseOutDuration(double);
    void setEaseInType(QEasingCurve::Type);
    void setEaseOutType(QEasingCurve::Type);

    // The curve at a point through the clip (0 = start, 1 = end).
    double valueAtProgress(double progress) const;

    float *process(float *value, uint size, double time) const override;
    QWidget *createPropertyEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double m_minimum = 0.0;
    double m_maximum = 1.0;
    double m_easeInDuration = 0.25;
    double m_easeOutDuration = 0.25;
    QEasingCurve::Type m_easeInType = QEasingCurve::Linear;
    QEasingCurve::Type m_easeOutType = QEasingCurve::Linear;
    QEasingCurve m_easingIn;
    QEasingCurve m_easingOut;
};

} // namespace photon

#endif // PHOTON_DURATIONCURVEEFFECT_H
