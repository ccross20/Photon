#ifndef PHOTON_SINEEFFECT_H
#define PHOTON_SINEEFFECT_H

#include <QGraphicsItem>
#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class SineEffect;


class SineEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    SineEffectEditor(SineEffect *);

private slots:
    void frequencyChanged(double);
    void amplitudeChanged(double);

protected:
    void relayout(const QRectF &) override;

private:
    SineEffect *m_effect;
    QPointF m_referencePt;
    QGraphicsRectItem *m_parentItem;
    RectangleGizmo *m_originHandle;
    RectangleGizmo *m_frequencyHandle;
    RectangleGizmo *m_amplitudeHandle;
    QGraphicsPathItem *m_pathItem;

};


class SineEffect : public ChannelEffect
{
public:
    SineEffect();

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

#endif // PHOTON_SINEEFFECT_H
