#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "noiseeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {




NoiseEffectEditor::NoiseEffectEditor(NoiseEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    QDoubleSpinBox *freqSpin = new QDoubleSpinBox;
    freqSpin->setMinimum(.001);
    freqSpin->setMaximum(9999);
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &QDoubleSpinBox::valueChanged, this, &NoiseEffectEditor::frequencyChanged);


    QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
    ampSpin->setMinimum(-255);
    ampSpin->setMaximum(255);
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &QDoubleSpinBox::valueChanged, this, &NoiseEffectEditor::amplitudeChanged);

    QSpinBox *seedSpin = new QSpinBox;
    seedSpin->setMinimum(0);
    seedSpin->setMaximum(9999);
    seedSpin->setValue(m_effect->seed());
    connect(seedSpin, &QSpinBox::valueChanged, this, &NoiseEffectEditor::seedChanged);

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


    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);

    m_frequencyHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, freqSpin](QPointF pt){
        m_effect->setFrequency(pt.x()/scale().x());
        freqSpin->setValue(m_effect->frequency());
    });
    m_frequencyHandle->setParentItem(m_parentItem);


    m_amplitudeHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, ampSpin](QPointF pt){
        m_effect->setAmplitude(pt.y()/scale().y());
        ampSpin->setValue(m_effect->amplitude());
    });
    m_amplitudeHandle->setOrientation(Qt::Vertical);
    m_amplitudeHandle->setParentItem(m_parentItem);

    m_originHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[](QPointF pt){

    });
    m_originHandle->setParentItem(m_parentItem);

    m_pathItem = new QGraphicsPathItem(m_parentItem);
    m_pathItem->setPen(QPen(Qt::cyan, 2));
    m_pathItem->setBrush(Qt::NoBrush);
    //addItem(m_originHandle);
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
    auto t = transform();

    double scaledFreq = m_effect->frequency();
    double startTime = m_effect->channel()->startTime();

    double x = startTime;

    if(t_sceneRect.left() > startTime)
    {
        x = (ceil((t_sceneRect.left() - startTime) / scaledFreq) * scaledFreq) + startTime;
    }

    m_referencePt = QPoint(x,0);

    m_parentItem->setPos(t.map(QPointF(x,0)));

    m_originHandle->setPos(QPointF(0,0));
    m_frequencyHandle->setPos(QPointF(scaledFreq * scale().x(),0));
    m_amplitudeHandle->setPos(QPointF(0, m_effect->amplitude() * scale().y()));

    QPainterPath path;
    path.moveTo(m_frequencyHandle->pos());
    path.lineTo(m_originHandle->pos());
    path.lineTo(m_amplitudeHandle->pos());
    m_pathItem->setPath(path);
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

double NoiseEffect::process(double value, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, time);
    }


    return value + m_noise.noise1D(time * (1000.0/m_frequency),0,m_amplitude);

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
