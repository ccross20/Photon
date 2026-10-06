#include "comparatoreffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "numberscrubfield.h"
#include "propertywidgets.h"

namespace photon {

ComparatorEffectEditor::ComparatorEffectEditor(ComparatorEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    auto *thresholdSpin = new photon::NumberScrubField;
    thresholdSpin->setValue(m_effect->threshold());
    connect(thresholdSpin, &photon::NumberScrubField::valueChanged, this, &ComparatorEffectEditor::thresholdChanged);

    auto *belowSpin = new photon::NumberScrubField;
    belowSpin->setValue(m_effect->belowValue());
    connect(belowSpin, &photon::NumberScrubField::valueChanged, this, &ComparatorEffectEditor::belowValueChanged);

    auto *aboveSpin = new photon::NumberScrubField;
    aboveSpin->setValue(m_effect->aboveValue());
    connect(aboveSpin, &photon::NumberScrubField::valueChanged, this, &ComparatorEffectEditor::aboveValueChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(thresholdSpin, "Threshold");
    paramWidget->addWidget(belowSpin, "Below Value");
    paramWidget->addWidget(aboveSpin, "Above Value");

    addWidget(paramWidget, "Comparator");
}

void ComparatorEffectEditor::thresholdChanged(double t_value)
{
    m_effect->setThreshold(t_value);
}

void ComparatorEffectEditor::belowValueChanged(double t_value)
{
    m_effect->setBelowValue(t_value);
}

void ComparatorEffectEditor::aboveValueChanged(double t_value)
{
    m_effect->setAboveValue(t_value);
}

EffectInformation ComparatorEffect::info()
{
    EffectInformation toReturn([](){return new ComparatorEffect;});
    toReturn.name = "Comparator";
    toReturn.effectId = "photon.effect.comparator";
    toReturn.categories.append("Modifier");

    return toReturn;
}

ComparatorEffect::ComparatorEffect() : ChannelEffect()
{
}

void ComparatorEffect::setThreshold(double t_value)
{
    m_threshold = t_value;
    updated();
}

void ComparatorEffect::setBelowValue(double t_value)
{
    m_belowValue = t_value;
    updated();
}

void ComparatorEffect::setAboveValue(double t_value)
{
    m_aboveValue = t_value;
    updated();
}

float *ComparatorEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
        value = previousEffect()->process(value, size, time);

    for(int i = 0; i < size; ++i)
        value[i] = static_cast<float>(value[i] >= m_threshold ? m_aboveValue : m_belowValue);

    return value;
}

ChannelEffectEditor *ComparatorEffect::createEditor()
{
    return new ComparatorEffectEditor(this);
}

QWidget *ComparatorEffect::createPropertyEditor()
{
    auto *form = new PropertyForm;
    form->addRow("Threshold", PropertyWidgets::createNumber(m_threshold, {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setThreshold(v); }));
    form->addRow("Below Value", PropertyWidgets::createNumber(m_belowValue, {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setBelowValue(v); }));
    form->addRow("Above Value", PropertyWidgets::createNumber(m_aboveValue, {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setAboveValue(v); }));
    return form;
}

void ComparatorEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("threshold"))
        m_threshold = t_json.value("threshold").toDouble();
    if(t_json.contains("belowValue"))
        m_belowValue = t_json.value("belowValue").toDouble();
    if(t_json.contains("aboveValue"))
        m_aboveValue = t_json.value("aboveValue").toDouble();
}

void ComparatorEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("threshold", m_threshold);
    t_json.insert("belowValue", m_belowValue);
    t_json.insert("aboveValue", m_aboveValue);
}

} // namespace photon
