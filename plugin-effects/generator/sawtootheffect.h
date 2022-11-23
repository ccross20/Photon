#ifndef PHOTON_SAWTOOTHEFFECT_H
#define PHOTON_SAWTOOTHEFFECT_H

#include "sequence/channeleffect.h"

namespace photon {

class SawtoothEffect;

class SawtoothEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    SawtoothEffectEditor(SawtoothEffect *);

private slots:
    void frequencyChanged(double);
    void amplitudeChanged(double);

private:
    SawtoothEffect *m_effect;
};


class SawtoothEffect : public ChannelEffect
{
public:
    SawtoothEffect();

    void setFrequency(double);
    void setAmplitude(double);
    double frequency() const{return m_frequency;}
    double amplitude() const{return m_amplitude;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();

private:
    double m_amplitude = 1.0;
    double m_frequency = 10.0;
};

} // namespace photon

#endif // PHOTON_SAWTOOTHEFFECT_H
