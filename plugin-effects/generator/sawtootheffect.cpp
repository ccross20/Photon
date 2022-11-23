

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include "sawtootheffect.h"

namespace photon {

SawtoothEffectEditor::SawtoothEffectEditor(SawtoothEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    setMaximumHeight(40);

    QDoubleSpinBox *freqSpin = new QDoubleSpinBox;
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &QDoubleSpinBox::valueChanged, this, &SawtoothEffectEditor::frequencyChanged);


    QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &QDoubleSpinBox::valueChanged, this, &SawtoothEffectEditor::amplitudeChanged);

}

void SawtoothEffectEditor::frequencyChanged(double t_value)
{
    m_effect->setFrequency(t_value);
}

void SawtoothEffectEditor::amplitudeChanged(double t_value)
{
    m_effect->setAmplitude(t_value);
}




EffectInformation SawtoothEffect::info()
{
    EffectInformation toReturn([](){return new SawtoothEffect;});
    toReturn.name = "Sawtooth";
    toReturn.effectId = "photon.effect.sawtooth";
    toReturn.categories.append("Generator");

    return toReturn;
}



SawtoothEffect::SawtoothEffect() : ChannelEffect()
{

}

void SawtoothEffect::setFrequency(double t_value)
{
    m_frequency = t_value;
    updated();

}

void SawtoothEffect::setAmplitude(double t_value)
{
    m_amplitude = t_value;
    updated();
}

double SawtoothEffect::process(double value, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, time);
    }

    double period = m_frequency;


    return value +  ((m_amplitude * 2.0) / period) * (period - abs(fmod(time, (2 * period)) - period)) - m_amplitude;
}

ChannelEffectEditor *SawtoothEffect::createEditor()
{
    return new SawtoothEffectEditor(this);
}

void SawtoothEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("amplitude"))
        m_amplitude = t_json.value("amplitude").toDouble();
    if(t_json.contains("frequency"))
        m_frequency = t_json.value("frequency").toDouble();
}

void SawtoothEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("amplitude", m_amplitude);
    t_json.insert("frequency", m_frequency);
}

} // namespace photon
