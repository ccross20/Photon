#include <QPen>
#include "numberscrubfield.h"
#include "propertywidgets.h"
#include "constantchanneleffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "gizmo/gizmohandle.h"

namespace photon {

ConstantEffectEditor::ConstantEffectEditor(ConstantChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);


    photon::NumberScrubField *constantSpin = new photon::NumberScrubField;
    constantSpin->setMinimum(-10000);
    constantSpin->setMaximum(10000);
    constantSpin->setSoftRange(-100,100);
    constantSpin->setValue(m_effect->value());
    connect(constantSpin, &photon::NumberScrubField::valueChanged, this, &ConstantEffectEditor::valueChanged);

    photon::NumberScrubField *rateSpin = new photon::NumberScrubField;
    rateSpin->setMinimum(-10000);
    rateSpin->setMaximum(10000);
    rateSpin->setSoftRange(-100,100);
    rateSpin->setValue(m_effect->rate()*100);
    connect(rateSpin, &photon::NumberScrubField::valueChanged, this, &ConstantEffectEditor::rateChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(constantSpin, "Value");
    paramWidget->addWidget(rateSpin, "Rate");

    addWidget(paramWidget, "Constant");

    m_gizmos = new GizmoGroup(scene(), this);

    // Value: drag vertically to set the constant level.
    m_valueHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Vertical);
    m_valueHandle->setDataGetter([this]{
        return QPointF(m_referenceTime, m_effect->value());
    });
    m_valueHandle->setDataSetter([this, constantSpin](QPointF pt){
        m_effect->setValue(pt.y());
        constantSpin->setValue(m_effect->value());
    });

    // Rate: sits five time-units to the right; its height above the value handle
    // is the slope over those five units, so dragging it sets the rate.
    m_rateHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Vertical);
    m_rateHandle->setDataGetter([this]{
        return QPointF(m_referenceTime + 5, m_effect->value() + m_effect->rate() * 5.0);
    });
    m_rateHandle->setDataSetter([this, rateSpin](QPointF pt){
        m_effect->setRate((pt.y() - m_effect->value()) / 5.0);
        rateSpin->setValue(m_effect->rate() * 100.0);
    });

    m_gizmos->connectLine(m_valueHandle, m_rateHandle);
}

void ConstantEffectEditor::valueChanged(double t_value)
{
    m_effect->setValue(t_value);
}

void ConstantEffectEditor::rateChanged(double t_value)
{
    m_effect->setRate(t_value/100.0);
}

void ConstantEffectEditor::relayout(const QRectF &t_sceneRect)
{
    // Anchor the handles to the horizontal centre of the visible range.
    m_referenceTime = t_sceneRect.center().x();
    m_gizmos->setTransform(transform());
}



EffectInformation ConstantChannelEffect::info()
{
    EffectInformation toReturn([](){return new ConstantChannelEffect;});
    toReturn.name = "Constant";
    toReturn.effectId = "photon.effect.constant";
    toReturn.categories.append("Generator");

    return toReturn;
}



ConstantChannelEffect::ConstantChannelEffect() : ChannelEffect()
{

}

void ConstantChannelEffect::setValue(double t_value)
{
    m_value = t_value;
    updated();
}

void ConstantChannelEffect::setRate(double t_value)
{
    m_rate = t_value;
    updated();
}

float * ConstantChannelEffect::process(float *value, uint size, double time) const
{

    for(int i = 0; i < size; ++i)
    {
        value[i] = m_value + (m_rate * time);
    }
    return value;
}

ChannelEffectEditor *ConstantChannelEffect::createEditor()
{
    return new ConstantEffectEditor(this);
}

QWidget *ConstantChannelEffect::createPropertyEditor()
{
    auto *form = new PropertyForm;
    form->addRow("Value", PropertyWidgets::createNumber(m_value, {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}}, [this](double v){ setValue(v); }));
    // Shown x100, matching the curve editor's rate handle and field.
    form->addRow("Rate", PropertyWidgets::createNumber(m_rate * 100.0, {{PropertyWidgets::MetaSoftMinimum, -100.0}, {PropertyWidgets::MetaSoftMaximum, 100.0}}, [this](double v){ setRate(v / 100.0); }));
    return form;
}

void ConstantChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_value = t_json.value("value").toDouble();
    m_rate = t_json.value("rate").toDouble();
}

void ConstantChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("value", m_value);
    t_json.insert("rate", m_rate);
}

} // namespace photon
