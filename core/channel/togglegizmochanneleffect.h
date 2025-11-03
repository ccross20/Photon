#ifndef TOGGLEGIZMOCHANNELEFFECT_H
#define TOGGLEGIZMOCHANNELEFFECT_H

#include "surface/gizmochanneleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class ToggleGizmoChannelEffect;

class ToggleGizmoChannelEffectEditor : public GizmoChannelEffectEditor
{
    Q_OBJECT
public:
    ToggleGizmoChannelEffectEditor(ToggleGizmoChannelEffect *);

private slots:
    void onValueChanged(double);
    void offValueChanged(double);

protected:
    void relayout(const QRectF &) override;

private:
    ToggleGizmoChannelEffect *m_effect;
    QGraphicsRectItem *m_parentItem;
    RectangleGizmo *m_originHandle;
    RectangleGizmo *m_rateHandle;
    QGraphicsPathItem *m_pathItem;

};


class ToggleGizmoChannelEffect : public GizmoChannelEffect
{
public:
    ToggleGizmoChannelEffect();

    void setOnValue(double);
    double onValue() const{return m_onValue;}
    void setOffValue(double);
    double offValue() const{return m_offValue;}
    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();



private:
    double m_onValue = 1.0;
    double m_offValue = 0;
};

} // namespace photon

#endif // TOGGLEGIZMOCHANNELEFFECT_H
