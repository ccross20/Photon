#ifndef STEPEFFECT_H
#define STEPEFFECT_H

#include "sequence/channeleffect.h"

namespace photon {

class StepEffect;

class StepEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    StepEffectEditor(StepEffect *);

private slots:
    void frequencyChanged(double);
    void amplitudeChanged(double);

private:
    StepEffect *m_effect;
};


class StepEffect : public ChannelEffect
{
public:
    StepEffect();

    void setFrequency(double);
    void setAmplitude(double);
    double frequency() const{return m_frequency;}
    double amplitude() const{return m_amplitude;}
    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();

private:
    double m_amplitude = 1.0;
    double m_frequency = 10.0;
};

} // namespace photon

#endif // STEPEFFECT_H
