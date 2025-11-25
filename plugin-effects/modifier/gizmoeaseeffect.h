#ifndef PHOTON_GIZMOEASEEFFECT_H
#define PHOTON_GIZMOEASEEFFECT_H

#include <QEasingCurve>
#include "surface/gizmochanneleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class GizmoEaseEffect;

class GizmoEaseEffectEditor : public GizmoChannelEffectEditor
{
    Q_OBJECT
public:
    GizmoEaseEffectEditor(GizmoEaseEffect *);

private slots:
    void easeOutDurationChanged(double);
    void easeInDurationChanged(double);
    void easeInChanged(int);
    void easeOutChanged(int);

protected:
    void relayout(const QRectF &) override;

private:
    GizmoEaseEffect *m_effect;
    QPointF m_referencePt;
    RectangleGizmo *m_startHandle;
    RectangleGizmo *m_endHandle;
    QGraphicsRectItem *m_parentItem;
    QGraphicsPathItem *m_pathItem;

};


class GizmoEaseEffect : public GizmoChannelEffect
{
public:
    GizmoEaseEffect();

    void setEaseInDuration(double);
    void setEaseOutDuration(double);
    void setEaseInType(QEasingCurve::Type);
    void setEaseOutType(QEasingCurve::Type);

    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;
    double easeInDuration() const{return m_easeInDuration;}
    double easeOutDuration() const{return m_easeOutDuration;}
    QEasingCurve::Type easeInType() const{return m_easeInType;}
    QEasingCurve::Type easeOutType() const{return m_easeOutType;}

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    QEasingCurve m_easingIn;
    QEasingCurve m_easingOut;
    QByteArray m_easeInEvent;
    QByteArray m_easeOutEvent;
    double m_easeInDuration = .5;
    double m_easeOutDuration = .5;
    QEasingCurve::Type m_easeInType = QEasingCurve::Linear;
    QEasingCurve::Type m_easeOutType = QEasingCurve::Linear;
};

} // namespace photon

#endif // PHOTON_GIZMOEASEEFFECT_H
