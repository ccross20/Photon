#ifndef PHOTON_COMPARATOREFFECT_H
#define PHOTON_COMPARATOREFFECT_H

#include "sequence/channeleffect.h"

namespace photon {

class ComparatorEffect;

class ComparatorEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    ComparatorEffectEditor(ComparatorEffect *);

private slots:
    void thresholdChanged(double);
    void belowValueChanged(double);
    void aboveValueChanged(double);

private:
    ComparatorEffect *m_effect;
};

// A comparator, same concept as in modular synths/analog circuits: a hard binary
// switch rather than a gate. It reads the chain's value so far and replaces it
// outright with one of two fixed values depending on which side of the threshold
// it falls on - unlike a noise gate, which mutes/passes the signal itself, a
// comparator's output never varies in magnitude, only which of the two constants
// comes out. Useful for turning a continuous signal (audio level, an LFO, ...)
// into a hard on/off switch for a channel.
class ComparatorEffect : public ChannelEffect
{
public:
    ComparatorEffect();

    void setThreshold(double);
    void setBelowValue(double);
    void setAboveValue(double);
    double threshold() const { return m_threshold; }
    double belowValue() const { return m_belowValue; }
    double aboveValue() const { return m_aboveValue; }

    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double m_threshold = 0.5;
    double m_belowValue = 0.0;
    double m_aboveValue = 1.0;
};

} // namespace photon

#endif // PHOTON_COMPARATOREFFECT_H
