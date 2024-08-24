#ifndef PHOTON_PULSEEFFECT_H
#define PHOTON_PULSEEFFECT_H

#include <QGraphicsItem>
#include <QEasingCurve>
#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class PulseEffect;

class PulseEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    PulseEffectEditor(PulseEffect *);

private slots:
    void frequencyChanged(double);
    void amplitudeChanged(double);
    void durationChanged(double);
    void timeOffsetChanged(double);
    void easeInDurationChanged(double);
    void easeOutDurationChanged(double);
    void easeInChanged(int);
    void easeOutChanged(int);

protected:
    void relayout(const QRectF &) override;

private:
    PulseEffect *m_effect;
    QPointF m_referencePt;
    QGraphicsRectItem *m_parentItem;
    RectangleGizmo *m_originHandle;
    RectangleGizmo *m_frequencyHandle;
    RectangleGizmo *m_durationHandle;
    RectangleGizmo *m_amplitudeHandle;
    QGraphicsPathItem *m_pathItem;

};


class PulseEffect : public ChannelEffect
{
public:
    PulseEffect();

    void setFrequency(double);
    void setAmplitude(double);
    void setDuration(double);
    void setOffset(double);
    void setEaseInDuration(double);
    void setEaseOutDuration(double);
    void setEaseInType(QEasingCurve::Type);
    void setEaseOutType(QEasingCurve::Type);
    double frequency() const{return m_frequency;}
    double amplitude() const{return m_amplitude;}
    double duration() const{return m_duration;}
    double offset() const{return m_offset;}
    double easeInDuration() const{return m_easeInDuration;}
    double easeOutDuration() const{return m_easeOutDuration;}
    QEasingCurve::Type easeInType() const{return m_easeInType;}
    QEasingCurve::Type easeOutType() const{return m_easeOutType;}
    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();


private:
    QEasingCurve m_easingIn;
    QEasingCurve m_easingOut;
    double m_amplitude = 1.0;
    double m_duration = .03;
    double m_offset = 0.0;
    double m_easeInDuration = .03;
    double m_easeOutDuration = .03;
    double m_frequency = 10.0;
    QEasingCurve::Type m_easeInType = QEasingCurve::Linear;
    QEasingCurve::Type m_easeOutType = QEasingCurve::Linear;
};

} // namespace photon

#endif // PHOTON_PULSEEFFECT_H
