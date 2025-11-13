#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include "stepeffect.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {

StepEffectEditor::StepEffectEditor(StepEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{

    QDoubleSpinBox *freqSpin = new QDoubleSpinBox;
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &QDoubleSpinBox::valueChanged, this, &StepEffectEditor::frequencyChanged);


    QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &QDoubleSpinBox::valueChanged, this, &StepEffectEditor::amplitudeChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(freqSpin, "Frequency");
    paramWidget->addWidget(ampSpin, "Amplitude");

    addWidget(paramWidget, "Step");

}

void StepEffectEditor::frequencyChanged(double t_value)
{
    m_effect->setFrequency(t_value);
}

void StepEffectEditor::amplitudeChanged(double t_value)
{
    m_effect->setAmplitude(t_value);
}




EffectInformation StepEffect::info()
{
    EffectInformation toReturn([](){return new StepEffect;});
    toReturn.name = "Step";
    toReturn.effectId = "photon.effect.step";
    toReturn.categories.append("Generator");

    return toReturn;
}



StepEffect::StepEffect() : ChannelEffect()
{

}

void StepEffect::setFrequency(double t_value)
{
    m_frequency = t_value;
    updated();

}

void StepEffect::setAmplitude(double t_value)
{
    m_amplitude = t_value;
    updated();
}

float *StepEffect::process(float *t_value, uint t_size, double t_time) const
{
    if(previousEffect())
    {
        t_value = previousEffect()->process(t_value, t_size, t_time);
    }

    double period = m_frequency;

    double t = fmod(t_time, (2 * period));


    for(int i = 0; i < t_size; ++i)
    {
        if(t > period)
            t_value[i] = t_value[i] - m_amplitude;
        else
            t_value[i] = t_value[i] + m_amplitude;

        //t_value[i] = t_value[i] +  ((m_amplitude * 2.0) / period) * (period - abs(fmod(t_time, (2 * period)) - period)) - m_amplitude;
    }

    return t_value;
}

ChannelEffectEditor *StepEffect::createEditor()
{
    return new StepEffectEditor(this);
}

void StepEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("amplitude"))
        m_amplitude = t_json.value("amplitude").toDouble();
    if(t_json.contains("frequency"))
        m_frequency = t_json.value("frequency").toDouble();
}

void StepEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("amplitude", m_amplitude);
    t_json.insert("frequency", m_frequency);
}

} // namespace photon
