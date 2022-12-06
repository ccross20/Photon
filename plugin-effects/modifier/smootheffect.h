#ifndef PHOTON_SMOOTHEFFECT_H
#define PHOTON_SMOOTHEFFECT_H

#include "sequence/channeleffect.h"

namespace photon {


class SmoothEffect;

class SmoothEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    SmoothEffectEditor(SmoothEffect *);

private slots:
    void samplesChanged(int);
    void spreadChanged(double);


private:
    SmoothEffect *m_effect;
};

class SmoothEffect : public ChannelEffect
{
public:
    SmoothEffect();

    void setSamples(int);
    void setSpread(double);
    int samples() const{return m_samples;}
    double spread() const{return m_spread;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    int m_samples = 5.0;
    double m_spread = 0.1;
};

} // namespace photon

#endif // PHOTON_SMOOTHEFFECT_H
