#include <QVBoxLayout>
#include <QLabel>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "sineeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "gui/gizmo/gizmohandle.h"
#include "view/numberscrubfield.h"

namespace photon {




SineEffectEditor::SineEffectEditor(SineEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    auto *freqSpin = new keira::NumberScrubField;
    freqSpin->setRange(.001, 9999);
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &keira::NumberScrubField::valueChanged, this, &SineEffectEditor::frequencyChanged);


    auto *ampSpin = new keira::NumberScrubField;
    ampSpin->setRange(-255, 255);
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &keira::NumberScrubField::valueChanged, this, &SineEffectEditor::amplitudeChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(freqSpin, "Frequency");
    paramWidget->addWidget(ampSpin, "Amplitude");

    addWidget(paramWidget, "Sinewave");

    // Handles are declared in data space (time, value); the group maps them to and
    // from the view transform, so there's no pixel math here.
    m_gizmos = new GizmoGroup(scene(), this);

    // Origin: a read-only marker at the start of the first visible period.
    m_originHandle = m_gizmos->addHandle();
    m_originHandle->setDataGetter([this]{ return QPointF(m_referenceTime, 0); });

    // Frequency: drag horizontally; the distance from the origin is the period.
    m_frequencyHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Horizontal);
    m_frequencyHandle->setDataGetter([this]{
        return QPointF(m_referenceTime + m_effect->frequency(), 0);
    });
    m_frequencyHandle->setDataSetter([this, freqSpin](QPointF pt){
        m_effect->setFrequency(std::max(.001, pt.x() - m_referenceTime));
        freqSpin->setValue(m_effect->frequency());
    });

    // Amplitude: drag vertically.
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
    // Anchor the gizmo to the first period boundary visible in the current view,
    // then let the group place every handle from its data getter.
    double freq = m_effect->frequency();
    double startTime = m_effect->channel()->startTime();

    double x = startTime;
    if(t_sceneRect.left() > startTime && freq > 0)
        x = (ceil((t_sceneRect.left() - startTime) / freq) * freq) + startTime;

    m_referenceTime = x;

    m_gizmos->setTransform(transform());
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
