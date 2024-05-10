#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QEasingCurve>
#include "pulseeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "sequence/sequence.h"

namespace photon {

PulseEffectEditor::PulseEffectEditor(PulseEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    QDoubleSpinBox *freqSpin = new QDoubleSpinBox;
    freqSpin->setMinimum(.001);
    freqSpin->setMaximum(9999);
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &QDoubleSpinBox::valueChanged, this, &PulseEffectEditor::frequencyChanged);


    QDoubleSpinBox *durationSpin = new QDoubleSpinBox;
    durationSpin->setMinimum(.001);
    durationSpin->setMaximum(9999);
    durationSpin->setValue(m_effect->duration());
    connect(durationSpin, &QDoubleSpinBox::valueChanged, this, &PulseEffectEditor::durationChanged);


    QDoubleSpinBox *offsetSpin = new QDoubleSpinBox;
    offsetSpin->setMinimum(-9999);
    offsetSpin->setMaximum(9999);
    offsetSpin->setValue(m_effect->offset());
    connect(offsetSpin, &QDoubleSpinBox::valueChanged, this, &PulseEffectEditor::timeOffsetChanged);


    QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
    ampSpin->setMinimum(-255);
    ampSpin->setMaximum(255);
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &QDoubleSpinBox::valueChanged, this, &PulseEffectEditor::amplitudeChanged);


    QDoubleSpinBox *easeInDurationSpin = new QDoubleSpinBox;
    easeInDurationSpin->setMinimum(.001);
    easeInDurationSpin->setMaximum(9999);
    easeInDurationSpin->setValue(m_effect->easeInDuration());
    connect(easeInDurationSpin, &QDoubleSpinBox::valueChanged, this, &PulseEffectEditor::easeInDurationChanged);

    QDoubleSpinBox *easeOutDurationSpin = new QDoubleSpinBox;
    easeOutDurationSpin->setMinimum(.001);
    easeOutDurationSpin->setMaximum(9999);
    easeOutDurationSpin->setValue(m_effect->easeOutDuration());
    connect(easeOutDurationSpin, &QDoubleSpinBox::valueChanged, this, &PulseEffectEditor::easeOutDurationChanged);

    QStringList easeStrings;
    easeStrings << "Linear";
    easeStrings << "In Quad";
    easeStrings << "Out Quad";
    easeStrings << "In Out Quad";
    easeStrings << "Out In Quad";

    easeStrings << "In Cubic";
    easeStrings << "Out Cubic";
    easeStrings << "In Out Cubic";
    easeStrings << "Out In Cubic";

    easeStrings << "In Quart";
    easeStrings << "Out Quart";
    easeStrings << "In Out Quart";
    easeStrings << "Out In Quart";

    easeStrings << "In Quint";
    easeStrings << "Out Quint";
    easeStrings << "In Out Quint";
    easeStrings << "Out In Quint";

    easeStrings << "In Sine";
    easeStrings << "Out Sine";
    easeStrings << "In Out Sine";
    easeStrings << "Out In Sine";

    easeStrings << "In Expo";
    easeStrings << "Out Expo";
    easeStrings << "In Out Expo";
    easeStrings << "Out In Expo";

    easeStrings << "In Circ";
    easeStrings << "Out Circ";
    easeStrings << "In Out Circ";
    easeStrings << "Out In Circ";

    easeStrings << "In Elastic";
    easeStrings << "Out Elastic";
    easeStrings << "In Out Elastic";
    easeStrings << "Out In Elastic";

    easeStrings << "In Back";
    easeStrings << "Out Back";
    easeStrings << "In Out Back";
    easeStrings << "Out In Back";

    easeStrings << "In Bounce";
    easeStrings << "Out Bounce";
    easeStrings << "In Out Bounce";
    easeStrings << "Out In Bounce";

    QComboBox *easeInCombo = new QComboBox;
    easeInCombo->addItems(easeStrings);
    easeInCombo->setCurrentIndex(m_effect->easeInType());
    connect(easeInCombo, &QComboBox::currentIndexChanged, this, &PulseEffectEditor::easeInChanged);

    QComboBox *easeOutCombo = new QComboBox;
    easeOutCombo->addItems(easeStrings);
    easeOutCombo->setCurrentIndex(m_effect->easeOutType());
    connect(easeOutCombo, &QComboBox::currentIndexChanged, this, &PulseEffectEditor::easeOutChanged);


    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(offsetSpin, "Offset");
    paramWidget->addWidget(freqSpin, "Frequency");
    paramWidget->addWidget(ampSpin, "Amplitude");
    paramWidget->addWidget(durationSpin, "Duration");
    paramWidget->addWidget(easeInDurationSpin, "Ease In Duration");
    paramWidget->addWidget(easeInCombo, "Ease In");
    paramWidget->addWidget(easeOutDurationSpin, "Ease Out Duration");
    paramWidget->addWidget(easeOutCombo, "Ease Out");

    addWidget(paramWidget, "Pulse");


    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);

    m_frequencyHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, freqSpin](QPointF pt){
        m_effect->setFrequency(pt.x()/scale().x());
        freqSpin->setValue(m_effect->frequency());
    });
    m_frequencyHandle->setParentItem(m_parentItem);


    m_durationHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, durationSpin](QPointF pt){
        m_effect->setDuration(pt.x()/scale().x());
        durationSpin->setValue(m_effect->duration());
    });
    m_durationHandle->setParentItem(m_parentItem);


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

void PulseEffectEditor::frequencyChanged(double t_value)
{
    m_effect->setFrequency(t_value);
}

void PulseEffectEditor::amplitudeChanged(double t_value)
{
    m_effect->setAmplitude(t_value);
}

void PulseEffectEditor::durationChanged(double t_value)
{
    m_effect->setDuration(t_value);
}

void PulseEffectEditor::timeOffsetChanged(double t_value)
{
    m_effect->setOffset(t_value);
}

void PulseEffectEditor::easeInDurationChanged(double t_value)
{
    m_effect->setEaseInDuration(t_value);
}

void PulseEffectEditor::easeOutDurationChanged(double t_value)
{
    m_effect->setEaseOutDuration(t_value);
}

void PulseEffectEditor::easeInChanged(int t_ease)
{
    m_effect->setEaseInType(static_cast<QEasingCurve::Type>(t_ease));
}

void PulseEffectEditor::easeOutChanged(int t_ease)
{
    m_effect->setEaseOutType(static_cast<QEasingCurve::Type>(t_ease));
}

void PulseEffectEditor::relayout(const QRectF &t_sceneRect)
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



EffectInformation PulseEffect::info()
{
    EffectInformation toReturn([](){return new PulseEffect;});
    toReturn.name = "Pulse";
    toReturn.effectId = "photon.effect.pulse";
    toReturn.categories.append("Generator");

    return toReturn;
}



PulseEffect::PulseEffect() : ChannelEffect()
{
    m_easingIn = QEasingCurve(QEasingCurve::InOutQuad);
    m_easingOut = QEasingCurve(QEasingCurve::InOutQuad);
}

void PulseEffect::setFrequency(double t_value)
{
    m_frequency = t_value;
    updated();

}

void PulseEffect::setAmplitude(double t_value)
{
    m_amplitude = t_value;
    updated();
}

void PulseEffect::setDuration(double t_value)
{
    m_duration = t_value;
    updated();
}

void PulseEffect::setOffset(double t_value)
{
    m_offset = t_value;
    updated();
}

void PulseEffect::setEaseInDuration(double t_value)
{
    m_easeInDuration = t_value;
    updated();
}

void PulseEffect::setEaseOutDuration(double t_value)
{
    m_easeOutDuration = t_value;
    updated();
}

void PulseEffect::setEaseInType(QEasingCurve::Type t_value)
{
    m_easeInType = t_value;
    m_easingIn.setType(t_value);
    updated();
}

void PulseEffect::setEaseOutType(QEasingCurve::Type t_value)
{
    m_easeOutType = t_value;
    m_easingOut.setType(t_value);
    updated();
}

QVariant PulseEffect::process(QVariant value, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, time);
    }


    double loopTime = m_frequency;
    double loopCount = std::floor((time - m_offset) / loopTime);
    double adjustedTime =  (time - m_offset) - (loopCount * loopTime);
    double gap = m_frequency - m_duration;

    if(time < m_offset)
        return value;

    if(adjustedTime > gap)
        return m_amplitude;

    double inDuration = m_easeInDuration;
    double outDuration = m_easeOutDuration;
    double totalDuration = inDuration + outDuration;

    if(totalDuration > gap)
    {
        inDuration = (m_easeInDuration / totalDuration) * gap;
        outDuration = (m_easeOutDuration / totalDuration) * gap;
    }

    double doubleVal = value.toDouble();

    if(adjustedTime > gap - inDuration)
    {
        return (m_easingIn.valueForProgress((adjustedTime - (gap - inDuration))/ inDuration) * (m_amplitude - doubleVal)) + doubleVal;
    }

    if(adjustedTime < outDuration && loopCount > 0)
    {
        return (m_easingOut.valueForProgress(1.0 - (adjustedTime / outDuration)) * (m_amplitude - doubleVal)) + doubleVal;
    }

    return value;
}

ChannelEffectEditor *PulseEffect::createEditor()
{
    return new PulseEffectEditor(this);
}

void PulseEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("amplitude"))
        m_amplitude = t_json.value("amplitude").toDouble();
    if(t_json.contains("frequency"))
        m_frequency = t_json.value("frequency").toDouble();
    if(t_json.contains("offset"))
        m_offset = t_json.value("offset").toDouble();
    if(t_json.contains("duration"))
        m_duration = t_json.value("duration").toDouble();
    if(t_json.contains("ease-in-duration"))
        m_easeInDuration = t_json.value("ease-in-duration").toDouble();
    if(t_json.contains("ease-out-duration"))
        m_easeOutDuration = t_json.value("ease-out-duration").toDouble();
    if(t_json.contains("ease-in-type"))
    {
        m_easeInType = static_cast<QEasingCurve::Type>(t_json.value("ease-in-type").toInt());
        m_easingIn.setType(m_easeInType);
    }
    if(t_json.contains("ease-out-type"))
    {
        m_easeOutType = static_cast<QEasingCurve::Type>(t_json.value("ease-out-type").toInt());
        m_easingOut.setType(m_easeOutType);
    }
}

void PulseEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("amplitude", m_amplitude);
    t_json.insert("frequency", m_frequency);
    t_json.insert("duration", m_duration);
    t_json.insert("offset", m_offset);
    t_json.insert("ease-in-duration", m_easeInDuration);
    t_json.insert("ease-out-duration", m_easeOutDuration);
    t_json.insert("ease-in-type", m_easeInType);
    t_json.insert("ease-out-type", m_easeOutType);
}

} // namespace photon
