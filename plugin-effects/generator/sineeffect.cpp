#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "sineeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {




SineEffectEditor::SineEffectEditor(SineEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    QDoubleSpinBox *freqSpin = new QDoubleSpinBox;
    freqSpin->setMinimum(.001);
    freqSpin->setMaximum(9999);
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &QDoubleSpinBox::valueChanged, this, &SineEffectEditor::frequencyChanged);


    QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
    ampSpin->setMinimum(-255);
    ampSpin->setMaximum(255);
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &QDoubleSpinBox::valueChanged, this, &SineEffectEditor::amplitudeChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(freqSpin, "Frequency");
    paramWidget->addWidget(ampSpin, "Amplitude");

    addWidget(paramWidget, "Sinewave");


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

void SineEffectEditor::frequencyChanged(double t_value)
{
    m_effect->setFrequency(t_value);
}

void SineEffectEditor::amplitudeChanged(double t_value)
{
    m_effect->setAmplitude(t_value);
}

void SineEffectEditor::relayout(const QRectF &t_sceneRect)
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



EffectInformation SineEffect::info()
{
    EffectInformation toReturn([](){return new SineEffect;});
    toReturn.name = "Sinewave";
    toReturn.effectId = "photon.effect.sinewave";
    toReturn.categories.append("Generator");

    return toReturn;
}



SineEffect::SineEffect() : ChannelEffect()
{

}

void SineEffect::setFrequency(double t_value)
{
    m_frequency = t_value;
    updated();

}

void SineEffect::setAmplitude(double t_value)
{
    m_amplitude = t_value;
    updated();
}

float *SineEffect::process(float *t_value, uint t_size, double t_time) const
{
    if(previousEffect())
    {
        t_value = previousEffect()->process(t_value, t_size, t_time);
    }

    for(int i = 0; i < t_size; ++i)
    {
        t_value[i] = t_value[i] + (std::sin(2 * M_PI * t_time * (1.0/m_frequency))*(m_amplitude));
    }

    return t_value;
}

ChannelEffectEditor *SineEffect::createEditor()
{
    return new SineEffectEditor(this);
}

void SineEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("amplitude"))
        m_amplitude = t_json.value("amplitude").toDouble();
    if(t_json.contains("frequency"))
        m_frequency = t_json.value("frequency").toDouble();
}

void SineEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("amplitude", m_amplitude);
    t_json.insert("frequency", m_frequency);
}

} // namespace photon
