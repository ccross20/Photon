#include <QHBoxLayout>
#include <QLabel>
#include <qmath.h>
#include <vector>
#include <algorithm>
#include "view/numberscrubfield.h"
#include "smootheffect.h"
#include "sequence/viewer/stackedparameterwidget.h"


namespace photon {

SmoothEffectEditor::SmoothEffectEditor(SmoothEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{

    auto *samplesSpin = new keira::NumberScrubField;
    samplesSpin->setIsInteger(true);
    samplesSpin->setMinimum(3);
    samplesSpin->setValue(m_effect->samples());
    connect(samplesSpin, &keira::NumberScrubField::valueChanged, this, [this](double v){ samplesChanged(int(v)); });


    auto *spreadSpin = new keira::NumberScrubField;
    spreadSpin->setMinimum(.0001);
    spreadSpin->setValue(m_effect->spread());
    connect(spreadSpin, &keira::NumberScrubField::valueChanged, this, &SmoothEffectEditor::spreadChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(samplesSpin, "Samples");
    paramWidget->addWidget(spreadSpin, "Spread");

    addWidget(paramWidget, "Smooth");
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
    ChannelEffect *prev = previousEffect();
    if(!prev)
        return value;

    const int samples = std::max(m_samples, 1);
    if(samples == 1)
        return prev->process(value, size, time);

    const double interval = m_spread / (samples - 1);
    const double halfSpread = m_spread / 2.0;

    // Gaussian kernel: taps near the centre count most, so a step edge smooths into
    // a rounded S-curve instead of a straight ramp. sigma = spread/6 puts the window
    // edges at +/-3 sigma, where the weight has effectively faded to zero.
    const double sigma = std::max(m_spread / 6.0, 1e-9);
    const double invTwoSigmaSq = 1.0 / (2.0 * sigma * sigma);

    // The incoming buffer is the seed the chain starts from. process() mutates its
    // input in place, so each tap must start from a fresh copy of that seed -
    // otherwise taps compound instead of being independent samples.
    std::vector<float> seed(value, value + size);
    std::vector<float> scratch(size);
    std::vector<double> sum(size, 0.0);
    double weightSum = 0.0;

    // The chain evaluation depends only on the tap's time, not on the element
    // index, so evaluate it once per tap (not once per element) and accumulate.
    for(int i = 0; i < samples; ++i)
    {
        const double offset = -halfSpread + interval * i;
        const double weight = std::exp(-(offset * offset) * invTwoSigmaSq);
        weightSum += weight;

        std::copy(seed.begin(), seed.end(), scratch.begin());
        float *result = prev->process(scratch.data(), size, time + offset);
        for(uint k = 0; k < size; ++k)
            sum[k] += weight * result[k];
    }

    const double invWeight = weightSum > 0.0 ? 1.0 / weightSum : 0.0;
    for(uint k = 0; k < size; ++k)
        value[k] = static_cast<float>(sum[k] * invWeight);

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
