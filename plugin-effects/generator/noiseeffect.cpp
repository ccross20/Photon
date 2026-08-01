#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "view/numberscrubfield.h"
#include "noiseeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "gui/gizmo/gizmohandle.h"

namespace photon {




NoiseEffectEditor::NoiseEffectEditor(NoiseEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    auto *freqSpin = new keira::NumberScrubField;
    freqSpin->setMinimum(.001);
    freqSpin->setMaximum(9999);
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &keira::NumberScrubField::valueChanged, this, &NoiseEffectEditor::frequencyChanged);


    auto *ampSpin = new keira::NumberScrubField;
    ampSpin->setMinimum(-255);
    ampSpin->setMaximum(255);
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &keira::NumberScrubField::valueChanged, this, &NoiseEffectEditor::amplitudeChanged);

    auto *seedSpin = new keira::NumberScrubField;
    seedSpin->setIsInteger(true);
    seedSpin->setMinimum(0);
    seedSpin->setMaximum(9999);
    seedSpin->setValue(m_effect->seed());
    connect(seedSpin, &keira::NumberScrubField::valueChanged, this, [this](double v){ seedChanged(int(v)); });

    QComboBox *typeCombo = new QComboBox;
    typeCombo->addItems({"Value","Value Fractal","Perlin","Perlin Fractal","Simplex","Simplex Fractal","Cellular","White Noise","Cubic","Cubic Fractal"});
    typeCombo->setCurrentIndex(m_effect->type());
    connect(typeCombo, &QComboBox::currentIndexChanged, this, &NoiseEffectEditor::typeChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(freqSpin, "Frequency");
    paramWidget->addWidget(ampSpin, "Amplitude");
    paramWidget->addWidget(seedSpin, "Seed");
    paramWidget->addWidget(typeCombo, "Type");


    addWidget(paramWidget, "Noise");

    m_gizmos = new GizmoGroup(scene(), this);

    m_originHandle = m_gizmos->addHandle();
    m_originHandle->setDataGetter([this]{ return QPointF(m_referenceTime, 0); });

    m_frequencyHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Horizontal);
    m_frequencyHandle->setDataGetter([this]{
        return QPointF(m_referenceTime + m_effect->frequency(), 0);
    });
    m_frequencyHandle->setDataSetter([this, freqSpin](QPointF pt){
        m_effect->setFrequency(std::max(.001, pt.x() - m_referenceTime));
        freqSpin->setValue(m_effect->frequency());
    });

    m_amplitudeHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Vertical);
    m_amplitudeHandle->setDataGetter([this]{
        return QPointF(m_referenceTime, m_effect->amplitude());
    });
    m_amplitudeHandle->setDataSetter([this, ampSpin](QPointF pt){
        m_effect->setAmplitude(pt.y());
        ampSpin->setValue(m_effect->amplitude());
    });

    m_gizmos->connectLine(m_frequencyHandle, m_originHandle);
    m_gizmos->connectLine(m_originHandle, m_amplitudeHandle);
}

void NoiseEffectEditor::seedChanged(int t_value)
{
    m_effect->setSeed(t_value);
}

void NoiseEffectEditor::typeChanged(int t_value)
{
    m_effect->setType(t_value);
}

void NoiseEffectEditor::frequencyChanged(double t_value)
{
    m_effect->setFrequency(t_value);
}

void NoiseEffectEditor::amplitudeChanged(double t_value)
{
    m_effect->setAmplitude(t_value);
}

void NoiseEffectEditor::relayout(const QRectF &t_sceneRect)
{
    double freq = m_effect->frequency();
    double startTime = m_effect->channel()->startTime();

    double x = startTime;
    if(t_sceneRect.left() > startTime && freq > 0)
        x = (ceil((t_sceneRect.left() - startTime) / freq) * freq) + startTime;

    m_referenceTime = x;

    m_gizmos->setTransform(transform());
}



EffectInformation NoiseEffect::info()
{
    EffectInformation toReturn([](){return new NoiseEffect;});
    toReturn.name = "Noise";
    toReturn.effectId = "photon.effect.noise";
    toReturn.categories.append("Generator");

    return toReturn;
}



NoiseEffect::NoiseEffect() : ChannelEffect(),m_noise(NoiseGenerator::NoiseMode1D)
{

}

void NoiseEffect::setFrequency(double t_value)
{
    m_frequency = t_value;
    updated();

}

void NoiseEffect::setAmplitude(double t_value)
{
    m_amplitude = t_value;
    updated();
}

void NoiseEffect::setSeed(int t_value)
{
    m_seed = t_value;
    m_noise.setSeed(m_seed);
    updated();
}

void NoiseEffect::setType(int t_value)
{
    m_noiseType = t_value;
    m_noise.setNoiseType(static_cast<NoiseGenerator::NoiseType>(t_value));
    updated();
}

float * NoiseEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, size, time);
    }

    for(int i = 0; i < size; ++i)
    {
        value[i] += m_noise.noise1D(time * (1000.0/m_frequency),0,m_amplitude);
    }

    return value;

}

ChannelEffectEditor *NoiseEffect::createEditor()
{
    return new NoiseEffectEditor(this);
}

void NoiseEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("amplitude"))
        m_amplitude = t_json.value("amplitude").toDouble();
    if(t_json.contains("frequency"))
        m_frequency = t_json.value("frequency").toDouble();
    if(t_json.contains("seed"))
        m_seed = t_json.value("seed").toDouble();
    if(t_json.contains("type"))
        m_noiseType = t_json.value("type").toDouble();

    m_noise.setNoiseType(static_cast<NoiseGenerator::NoiseType>(m_noiseType));
    m_noise.setSeed(m_seed);
}

void NoiseEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("amplitude", m_amplitude);
    t_json.insert("frequency", m_frequency);
    t_json.insert("seed", m_seed);
    t_json.insert("type", m_noiseType);

}

} // namespace photon
