#ifndef PHOTON_NOISEEFFECT_H
#define PHOTON_NOISEEFFECT_H

#include <QGraphicsItem>
#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"
#include "util/noisegenerator.h"

namespace photon {

class NoiseEffect;


class NoiseEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    NoiseEffectEditor(NoiseEffect *);

private slots:
    void frequencyChanged(double);
    void amplitudeChanged(double);
    void seedChanged(int);
    void typeChanged(int);

protected:
    void relayout(const QRectF &) override;

private:
    NoiseEffect *m_effect;
    QPointF m_referencePt;
    QGraphicsRectItem *m_parentItem;
    RectangleGizmo *m_originHandle;
    RectangleGizmo *m_frequencyHandle;
    RectangleGizmo *m_amplitudeHandle;
    QGraphicsPathItem *m_pathItem;

};


class NoiseEffect : public ChannelEffect
{
public:
    NoiseEffect();

    void setFrequency(double);
    void setAmplitude(double);
    void setSeed(int);
    void setType(int);
    double frequency() const{return m_frequency;}
    double amplitude() const{return m_amplitude;}
    int seed() const{return m_seed;}
    int type() const{return m_noiseType;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();


private:
    NoiseGenerator m_noise;
    int m_seed = 0;
    int m_noiseType = 0;
    double m_amplitude = 1.0;
    double m_frequency = 10.0;
};

} // namespace photon


#endif // PHOTON_NOISEEFFECT_H
