#ifndef PHOTON_LOOPEFFECT_H
#define PHOTON_LOOPEFFECT_H

#include "sequence/channeleffect.h"

namespace photon {

class LoopEffect;

class LoopEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    LoopEffectEditor(LoopEffect *);

private slots:
    void durationChanged(double);
    void mirrorChanged(bool);

private:
    LoopEffect *m_effect;
};


class LoopEffect : public ChannelEffect
{
public:
    LoopEffect();

    void setDuration(double);
    void setMirror(bool);
    double duration() const{return m_duration;}
    bool mirror() const{return m_mirror;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();

private:
    double m_duration = 2.0;
    bool m_mirror = false;
};

} // namespace photon

#endif // PHOTON_LOOPEFFECT_H
