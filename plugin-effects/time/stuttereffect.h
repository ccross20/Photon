#ifndef PHOTON_STUTTEREFFECT_H
#define PHOTON_STUTTEREFFECT_H

#include "sequence/channeleffect.h"

namespace photon {


class StutterEffect;

class StutterEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    StutterEffectEditor(StutterEffect *);

private slots:
    void gapChanged(double);
    void durationChanged(double);

private:
    StutterEffect *m_effect;
};

class StutterEffect : public ChannelEffect
{
public:
    StutterEffect();

    void setGap(double);
    void setDuration(double);
    double gap() const{return m_gap;}
    double duration() const{return m_duration;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double m_gap = 1.0;
    double m_duration = 1.0;
};

} // namespace photon

#endif // PHOTON_STUTTEREFFECT_H
