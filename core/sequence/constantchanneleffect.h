#ifndef PHOTON_CONSTANTCHANNELEFFECT_H
#define PHOTON_CONSTANTCHANNELEFFECT_H

#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class ConstantChannelEffect;

class ConstantEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    ConstantEffectEditor(ConstantChannelEffect *);

private slots:
    void valueChanged(double);

protected:
    void relayout(const QRectF &) override;

private:
    ConstantChannelEffect *m_effect;
    RectangleGizmo *m_originHandle;
    QGraphicsPathItem *m_pathItem;

};


class ConstantChannelEffect : public ChannelEffect
{
public:
    ConstantChannelEffect();

    void setValue(double);
    double value() const{return m_value;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();


private:
    double m_value = 1.0;
};

} // namespace photon

#endif // PHOTON_CONSTANTCHANNELEFFECT_H
