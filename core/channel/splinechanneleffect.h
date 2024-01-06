#ifndef PHOTON_SPLINECHANNELEFFECT_H
#define PHOTON_SPLINECHANNELEFFECT_H

#include "data/spline.h"
#include "sequence/channeleffect.h"

namespace photon {

class SplineChannelEffect;

class SplineEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    SplineEffectEditor(SplineChannelEffect *);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    SplineChannelEffect *m_effect;
};

class SplineChannelEffect : public ChannelEffect
{
public:
    SplineChannelEffect();

    Spline value() const;
    void setValue(const Spline &);
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();
private:
    Spline m_spline;
};

} // namespace photon

#endif // PHOTON_SPLINECHANNELEFFECT_H
