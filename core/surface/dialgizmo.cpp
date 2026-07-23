#include "dialgizmo.h"

namespace photon {

const QByteArray DialGizmo::GizmoId = "Dial";

DialGizmo::DialGizmo():SurfaceGizmo("Dial")
{
    addProperty("text",       "Label",             GizmoProperty::Text,    QString("Dial"));
    addProperty("min",        "Min",               GizmoProperty::Number,  0.0);
    addProperty("max",        "Max",               GizmoProperty::Number,  1.0);
    addProperty("value",      "Value",             GizmoProperty::Number,  0.0);
    addProperty("continuous", "Infinite Rotation", GizmoProperty::Boolean, false);
}

QString DialGizmo::text() const
{
    return propertyValue("text").toString();
}

void DialGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

double DialGizmo::minValue() const
{
    return propertyValue("min").toDouble();
}

void DialGizmo::setMinValue(double t_value)
{
    setPropertyValue("min", t_value);
}

double DialGizmo::maxValue() const
{
    return propertyValue("max").toDouble();
}

void DialGizmo::setMaxValue(double t_value)
{
    setPropertyValue("max", t_value);
}

double DialGizmo::value() const
{
    return propertyValue("value").toDouble();
}

void DialGizmo::setValue(double t_value)
{
    setPropertyValue("value", t_value);
}

bool DialGizmo::isContinuous() const
{
    return propertyValue("continuous").toBool();
}

void DialGizmo::setIsContinuous(bool t_value)
{
    setPropertyValue("continuous", t_value);
}

QVector<SurfaceGizmo::GizmoOutput> DialGizmo::outputs() const
{
    return { {"value", "Value", GizmoProperty::Number} };
}

QVariant DialGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "value")
        return value();
    return {};
}

} // namespace photon
