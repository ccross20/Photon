#ifndef PHOTON_SINEEFFECT_H
#define PHOTON_SINEEFFECT_H

#include <QGraphicsItem>
#include "sequence/channeleffect.h"
#include "gui/gizmo/gizmogroup.h"

namespace photon {

class SineEffect;
class GizmoGroup;
class GizmoHandle;


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
    double m_referenceTime = 0;
    GizmoGroup *m_gizmos;
    GizmoHandle *m_originHandle;
    GizmoHandle *m_frequencyHandle;
    GizmoHandle *m_amplitudeHandle;

};


class SineEffect : public ChannelEffect
{
public:
    SineEffect();

    void setFrequency(double);
    void setAmplitude(double);
    double frequency() const{return m_frequency;}
    double amplitude() const{return m_amplitude;}
    float *process(float *value, uint size, double time) const override;
    bool providesIsolatedContribution() const override { return true; }
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
