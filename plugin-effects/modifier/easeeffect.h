#ifndef PHOTON_EASEEFFECT_H
#define PHOTON_EASEEFFECT_H

#include <QEasingCurve>
#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class EaseEffect;

class EaseEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    EaseEffectEditor(EaseEffect *);

private slots:
    void easeOutDurationChanged(double);
    void easeInDurationChanged(double);
    void easeInChanged(int);
    void easeOutChanged(int);

protected:
    void relayout(const QRectF &) override;

private:
    EaseEffect *m_effect;
    QPointF m_referencePt;
    RectangleGizmo *m_startHandle;
    RectangleGizmo *m_endHandle;
    QGraphicsRectItem *m_parentItem;
    QGraphicsPathItem *m_pathItem;

};


class EaseEffect : public ChannelEffect
{
public:
    EaseEffect();

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
    double m_easeInDuration = .5;
    double m_easeOutDuration = .5;
    QEasingCurve::Type m_easeInType = QEasingCurve::Linear;
    QEasingCurve::Type m_easeOutType = QEasingCurve::Linear;
};

} // namespace photon

#endif // PHOTON_EASEEFFECT_H
