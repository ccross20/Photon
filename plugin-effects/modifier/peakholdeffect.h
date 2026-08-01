#ifndef PHOTON_PEAKHOLDEFFECT_H
#define PHOTON_PEAKHOLDEFFECT_H

#include <QEasingCurve>
#include "sequence/channeleffect.h"

namespace photon {

class PeakHoldEffect;

class PeakHoldEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    PeakHoldEffectEditor(PeakHoldEffect *);

private slots:
    void thresholdChanged(double);
    void holdTimeChanged(double);
    void decayTimeChanged(double);
    void decayCurveChanged(int);

private:
    PeakHoldEffect *m_effect;
};

// The VU-meter "peak hold": when the incoming signal crosses up through a
// threshold (e.g. a one-frame spike to 1 on an otherwise-0 signal), this holds
// the peak value it saw for holdTime, then eases it back down to 0 over
// decayTime. A later spike always supersedes an earlier one still decaying -
// retriggering just restarts the hold from the new peak.
//
// Unlike a real envelope follower, this can't track running state: process(time)
// is queried at arbitrary times, not just forward in playback order (the curve
// editor samples the whole clip to draw it, scrubbing jumps around). So instead
// each query walks the upstream chain backward in small steps over the last
// (holdTime + decayTime) to find the most recent threshold crossing - the
// trigger this instant's output should be following - bounded by maxScanSteps
// so a long hold+decay can't blow up the cost of a single query.
class PeakHoldEffect : public ChannelEffect
{
public:
    PeakHoldEffect();

    void setThreshold(double);
    void setHoldTime(double);
    void setDecayTime(double);
    void setDecayCurveType(QEasingCurve::Type);
    double threshold() const { return m_threshold; }
    double holdTime() const { return m_holdTime; }
    double decayTime() const { return m_decayTime; }
    QEasingCurve::Type decayCurveType() const { return m_decayCurveType; }

    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double m_threshold = 0.5;
    double m_holdTime = 0.25;
    double m_decayTime = 1.0;
    QEasingCurve::Type m_decayCurveType = QEasingCurve::OutQuad;
    QEasingCurve m_decayCurve;
};

} // namespace photon

#endif // PHOTON_PEAKHOLDEFFECT_H
