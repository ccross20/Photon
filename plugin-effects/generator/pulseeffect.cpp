#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "view/numberscrubfield.h"
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QEasingCurve>
#include "pulseeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "sequence/sequence.h"
#include "gui/gizmo/gizmohandle.h"
#include "util/utils.h"

namespace photon {

PulseEffectEditor::PulseEffectEditor(PulseEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    keira::NumberScrubField *freqSpin = new keira::NumberScrubField;
    freqSpin->setMinimum(.001);
    freqSpin->setMaximum(9999);
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &keira::NumberScrubField::valueChanged, this, &PulseEffectEditor::frequencyChanged);


    keira::NumberScrubField *durationSpin = new keira::NumberScrubField;
    durationSpin->setMinimum(.001);
    durationSpin->setMaximum(9999);
    durationSpin->setValue(m_effect->duration());
    connect(durationSpin, &keira::NumberScrubField::valueChanged, this, &PulseEffectEditor::durationChanged);


    keira::NumberScrubField *offsetSpin = new keira::NumberScrubField;
    offsetSpin->setMinimum(-9999);
    offsetSpin->setMaximum(9999);
    offsetSpin->setValue(m_effect->offset());
    connect(offsetSpin, &keira::NumberScrubField::valueChanged, this, &PulseEffectEditor::timeOffsetChanged);


    keira::NumberScrubField *ampSpin = new keira::NumberScrubField;
    ampSpin->setMinimum(-255);
    ampSpin->setMaximum(255);
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &keira::NumberScrubField::valueChanged, this, &PulseEffectEditor::amplitudeChanged);


    keira::NumberScrubField *easeInDurationSpin = new keira::NumberScrubField;
    easeInDurationSpin->setMinimum(.001);
    easeInDurationSpin->setMaximum(9999);
    easeInDurationSpin->setValue(m_effect->easeInDuration());
    connect(easeInDurationSpin, &keira::NumberScrubField::valueChanged, this, &PulseEffectEditor::easeInDurationChanged);

    keira::NumberScrubField *easeOutDurationSpin = new keira::NumberScrubField;
    easeOutDurationSpin->setMinimum(.001);
    easeOutDurationSpin->setMaximum(9999);
    easeOutDurationSpin->setValue(m_effect->easeOutDuration());
    connect(easeOutDurationSpin, &keira::NumberScrubField::valueChanged, this, &PulseEffectEditor::easeOutDurationChanged);

    QStringList easeList = easeStrings();

    QComboBox *easeInCombo = new QComboBox;
    easeInCombo->addItems(easeList);
    easeInCombo->setCurrentIndex(m_effect->easeInType());
    connect(easeInCombo, &QComboBox::currentIndexChanged, this, &PulseEffectEditor::easeInChanged);

    QComboBox *easeOutCombo = new QComboBox;
    easeOutCombo->addItems(easeList);
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

    m_durationHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Horizontal);
    m_durationHandle->setDataGetter([this]{
        return QPointF(m_referenceTime + m_effect->duration(), 0);
    });
    m_durationHandle->setDataSetter([this, durationSpin](QPointF pt){
        m_effect->setDuration(std::max(.001, pt.x() - m_referenceTime));
        durationSpin->setValue(m_effect->duration());
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
    double freq = m_effect->frequency();
    double startTime = m_effect->channel()->startTime();

    double x = startTime;
    if(t_sceneRect.left() > startTime && freq > 0)
        x = (ceil((t_sceneRect.left() - startTime) / freq) * freq) + startTime;

    m_referenceTime = x;

    m_gizmos->setTransform(transform());
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

float * PulseEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, size, time);
    }


    double loopTime = m_frequency;
    double loopCount = std::floor((time - m_offset) / loopTime);
    double adjustedTime =  (time - m_offset) - (loopCount * loopTime);
    double gap = m_frequency - m_duration;

    if(time < m_offset)
        return value;


    for(int i = 0; i < size; ++i)
    {
        if(adjustedTime > gap)
        {
            value[i] = m_amplitude;
            continue;
        }

        double inDuration = m_easeInDuration;
        double outDuration = m_easeOutDuration;
        double totalDuration = inDuration + outDuration;

        if(totalDuration > gap)
        {
            inDuration = (m_easeInDuration / totalDuration) * gap;
            outDuration = (m_easeOutDuration / totalDuration) * gap;
        }

        double doubleVal = value[i];

        if(adjustedTime > gap - inDuration)
        {
            value[i] = (m_easingIn.valueForProgress((adjustedTime - (gap - inDuration))/ inDuration) * (m_amplitude - doubleVal)) + doubleVal;
            continue;
        }

        if(adjustedTime < outDuration && loopCount > 0)
        {
            value[i] = (m_easingOut.valueForProgress(1.0 - (adjustedTime / outDuration)) * (m_amplitude - doubleVal)) + doubleVal;
            continue;
        }

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
