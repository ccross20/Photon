#ifndef PHOTON_CHANNELREMAPEFFECT_H
#define PHOTON_CHANNELREMAPEFFECT_H

#include <QEasingCurve>
#include "sequence/channeleffect.h"

namespace photon {

class ChannelRemapEffect;

class ChannelRemapEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    ChannelRemapEffectEditor(ChannelRemapEffect *);

private slots:
    void minInputChanged(double);
    void maxInputChanged(double);
    void minOutputChanged(double);
    void maxOutputChanged(double);
    void easeTypeChanged(int);
    void boundsModeChanged(int);

private:
    ChannelRemapEffect *m_effect;
};

// Same remapping behaviour as the "Remap" routine/math node (photon.math.remap):
// normalize the incoming value against [minInput, maxInput], apply a bounds mode
// for values outside that range (Limit clamps, Loop wraps, Ping Pong reflects),
// ease the normalized position, then scale into [minOutput, maxOutput].
class ChannelRemapEffect : public ChannelEffect
{
public:
    enum BoundsMode
    {
        BoundsLimit,
        BoundsLoop,
        BoundsPingPong
    };

    ChannelRemapEffect();

    void setMinInput(double);
    void setMaxInput(double);
    void setMinOutput(double);
    void setMaxOutput(double);
    void setEaseType(QEasingCurve::Type);
    void setBoundsMode(BoundsMode);

    double minInput() const { return m_minInput; }
    double maxInput() const { return m_maxInput; }
    double minOutput() const { return m_minOutput; }
    double maxOutput() const { return m_maxOutput; }
    QEasingCurve::Type easeType() const { return m_easeType; }
    BoundsMode boundsMode() const { return m_boundsMode; }

    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double m_minInput = 0.0;
    double m_maxInput = 1.0;
    double m_minOutput = 0.0;
    double m_maxOutput = 1.0;
    QEasingCurve::Type m_easeType = QEasingCurve::Linear;
    QEasingCurve m_easing;
    BoundsMode m_boundsMode = BoundsLimit;
};

} // namespace photon

#endif // PHOTON_CHANNELREMAPEFFECT_H
