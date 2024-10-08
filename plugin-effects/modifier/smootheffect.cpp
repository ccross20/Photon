#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <qmath.h>
#include "smootheffect.h"
#include "sequence/viewer/stackedparameterwidget.h"


namespace photon {

SmoothEffectEditor::SmoothEffectEditor(SmoothEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{

    QSpinBox *samplesSpin = new QSpinBox;
    samplesSpin->setMinimum(3);
    samplesSpin->setValue(m_effect->samples());
    connect(samplesSpin, &QSpinBox::valueChanged, this, &SmoothEffectEditor::samplesChanged);


    QDoubleSpinBox *spreadSpin = new QDoubleSpinBox;
    spreadSpin->setMinimum(.0001);
    spreadSpin->setValue(m_effect->spread());
    connect(spreadSpin, &QDoubleSpinBox::valueChanged, this, &SmoothEffectEditor::spreadChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(samplesSpin, "Samples");
    paramWidget->addWidget(spreadSpin, "Spread");

    addWidget(paramWidget, "Sinewave");
}

void SmoothEffectEditor::samplesChanged(int t_value)
{
    m_effect->setSamples(t_value);
}

void SmoothEffectEditor::spreadChanged(double t_value)
{
    m_effect->setSpread(t_value);
}





EffectInformation SmoothEffect::info()
{
    EffectInformation toReturn([](){return new SmoothEffect;});
    toReturn.name = "Smooth";
    toReturn.effectId = "photon.effect.smooth";
    toReturn.categories.append("Modifier");

    return toReturn;
}



SmoothEffect::SmoothEffect() : ChannelEffect()
{

}

void SmoothEffect::setSamples(int t_value)
{
    m_samples = t_value;
    updated();

}

void SmoothEffect::setSpread(double t_value)
{
    m_spread = t_value;
    updated();
}

float * SmoothEffect::process(float *value, uint size, double time) const
{

    if(previousEffect())
    {
        double interval = m_spread / (m_samples-1);
        double halfSpread = m_spread / 2.0;

        for(int k = 0; k < size; ++k)
        {

            double total = 0.0;

            for(int i = 0; i < m_samples; ++i)
            {
                total += previousEffect()->process(value, size, time - halfSpread + (interval * i))[k];
            }


            value[k] = total / static_cast<double>(m_samples);
        }


    }


    return value;
}

ChannelEffectEditor *SmoothEffect::createEditor()
{
    return new SmoothEffectEditor(this);
}

void SmoothEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("samples"))
        m_samples = t_json.value("samples").toDouble();
    if(t_json.contains("spread"))
        m_spread = t_json.value("spread").toDouble();
}

void SmoothEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("samples", m_samples);
    t_json.insert("spread", m_spread);
}

} // namespace photon
