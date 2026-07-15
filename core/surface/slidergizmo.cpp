#include "slidergizmo.h"

namespace photon {

const QByteArray SliderGizmo::GizmoId = "Slider";

SliderGizmo::SliderGizmo():SurfaceGizmo("Slider")
{
    addProperty("text", "Label",  GizmoProperty::Text,   QString("Slider"));
    addProperty("min",  "Min",    GizmoProperty::Number, 0.0);
    addProperty("max",  "Max",    GizmoProperty::Number, 1.0);
    addProperty("value","Value",  GizmoProperty::Number, 0.0);
}

QString SliderGizmo::text() const
{
    return propertyValue("text").toString();
}
void SliderGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

double SliderGizmo::minValue() const
{
    return propertyValue("min").toDouble();
}

void SliderGizmo::setMinValue(double t_value)
{
    setPropertyValue("min", t_value);
}

double SliderGizmo::maxValue() const
{
    return propertyValue("max").toDouble();
}

void SliderGizmo::setMaxValue(double t_value)
{
    setPropertyValue("max", t_value);
}

double SliderGizmo::value() const
{
    return propertyValue("value").toDouble();
}

void SliderGizmo::setValue(double t_value)
{
    setPropertyValue("value", t_value);
}

QVector<SurfaceGizmo::GizmoOutput> SliderGizmo::outputs() const
{
    return { {"value", "Value", GizmoProperty::Number} };
}

QVariant SliderGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "value")
        return value();
    return {};
}


} // namespace photon
