#include <algorithm>
#include <QJsonObject>
#include "durationcurveeffect.h"
#include "sequence/channel.h"
#include "propertywidgets.h"
#include "util/utils.h"

namespace photon {

EffectInformation DurationCurveEffect::info()
{
    EffectInformation toReturn([](){return new DurationCurveEffect;});
    toReturn.name = "Duration Curve";
    toReturn.effectId = "photon.effect.duration-curve";
    toReturn.categories.append("Generator");
    return toReturn;
}

DurationCurveEffect::DurationCurveEffect()
{
    m_easingIn.setType(m_easeInType);
    m_easingOut.setType(m_easeOutType);
}

void DurationCurveEffect::setMinimum(double t_value)
{
    m_minimum = t_value;
    updated();
}

void DurationCurveEffect::setMaximum(double t_value)
{
    m_maximum = t_value;
    updated();
}

void DurationCurveEffect::setEaseInDuration(double t_value)
{
    m_easeInDuration = std::clamp(t_value, 0.0, 1.0);
    updated();
}

void DurationCurveEffect::setEaseOutDuration(double t_value)
{
    m_easeOutDuration = std::clamp(t_value, 0.0, 1.0);
    updated();
}

void DurationCurveEffect::setEaseInType(QEasingCurve::Type t_value)
{
    m_easeInType = t_value;
    m_easingIn.setType(t_value);
    updated();
}

void DurationCurveEffect::setEaseOutType(QEasingCurve::Type t_value)
{
    m_easeOutType = t_value;
    m_easingOut.setType(t_value);
    updated();
}

double DurationCurveEffect::valueAtProgress(double t_progress) const
{
    const double progress = std::clamp(t_progress, 0.0, 1.0);

    // Shares of the clip; scaled down together when they'd overlap.
    double easeIn = m_easeInDuration;
    double easeOut = m_easeOutDuration;
    const double total = easeIn + easeOut;
    if(total > 1.0)
    {
        easeIn /= total;
        easeOut /= total;
    }

    double amount = 1.0;
    if(easeIn > 0.0 && progress < easeIn)
        amount = m_easingIn.valueForProgress(progress / easeIn);
    else if(easeOut > 0.0 && progress > 1.0 - easeOut)
        amount = m_easingOut.valueForProgress((1.0 - progress) / easeOut);

    return m_minimum + (m_maximum - m_minimum) * amount;
}

float *DurationCurveEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
        value = previousEffect()->process(value, size, time);

    // `time` is relative to the channel's start; the channel spans the clip.
    const double duration = channel() ? channel()->duration() : 0.0;
    const double progress = duration > 0.0 ? time / duration : 0.5;
    const float result = static_cast<float>(valueAtProgress(progress));

    for(uint i = 0; i < size; ++i)
        value[i] = result;
    return value;
}

QWidget *DurationCurveEffect::createPropertyEditor()
{
    auto *form = new PropertyForm;
    form->addRow("Min", PropertyWidgets::createNumber(m_minimum,
        {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setMinimum(v); }));
    form->addRow("Max", PropertyWidgets::createNumber(m_maximum,
        {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setMaximum(v); }));
    form->addRow("Ease In", PropertyWidgets::createOptions(easeStrings(), m_easeInType, {},
        [this](int v){ setEaseInType(static_cast<QEasingCurve::Type>(v)); }));
    form->addRow("Ease In Duration", PropertyWidgets::createNumber(m_easeInDuration,
        {{PropertyWidgets::MetaMinimum, 0.0}, {PropertyWidgets::MetaMaximum, 1.0}},
        [this](double v){ setEaseInDuration(v); }));
    form->addRow("Ease Out", PropertyWidgets::createOptions(easeStrings(), m_easeOutType, {},
        [this](int v){ setEaseOutType(static_cast<QEasingCurve::Type>(v)); }));
    form->addRow("Ease Out Duration", PropertyWidgets::createNumber(m_easeOutDuration,
        {{PropertyWidgets::MetaMinimum, 0.0}, {PropertyWidgets::MetaMaximum, 1.0}},
        [this](double v){ setEaseOutDuration(v); }));
    return form;
}

void DurationCurveEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_minimum = t_json.value("minimum").toDouble(m_minimum);
    m_maximum = t_json.value("maximum").toDouble(m_maximum);
    m_easeInDuration = std::clamp(t_json.value("ease-in-duration").toDouble(m_easeInDuration), 0.0, 1.0);
    m_easeOutDuration = std::clamp(t_json.value("ease-out-duration").toDouble(m_easeOutDuration), 0.0, 1.0);
    m_easeInType = static_cast<QEasingCurve::Type>(t_json.value("ease-in-type").toInt(m_easeInType));
    m_easeOutType = static_cast<QEasingCurve::Type>(t_json.value("ease-out-type").toInt(m_easeOutType));
    m_easingIn.setType(m_easeInType);
    m_easingOut.setType(m_easeOutType);
}

void DurationCurveEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("minimum", m_minimum);
    t_json.insert("maximum", m_maximum);
    t_json.insert("ease-in-duration", m_easeInDuration);
    t_json.insert("ease-out-duration", m_easeOutDuration);
    t_json.insert("ease-in-type", int(m_easeInType));
    t_json.insert("ease-out-type", int(m_easeOutType));
}

} // namespace photon
