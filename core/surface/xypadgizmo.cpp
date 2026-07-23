#include "xypadgizmo.h"

namespace photon {

const QByteArray XYPadGizmo::GizmoId = "XYPad";

XYPadGizmo::XYPadGizmo():SurfaceGizmo("XYPad")
{
    addProperty("text",   "Label", GizmoProperty::Text,   QString("XY Pad"));
    addProperty("minX",   "Min X", GizmoProperty::Number, 0.0);
    addProperty("maxX",   "Max X", GizmoProperty::Number, 1.0);
    addProperty("valueX", "X",     GizmoProperty::Number, 0.5);
    addProperty("minY",   "Min Y", GizmoProperty::Number, 0.0);
    addProperty("maxY",   "Max Y", GizmoProperty::Number, 1.0);
    addProperty("valueY", "Y",     GizmoProperty::Number, 0.5);
}

QString XYPadGizmo::text() const
{
    return propertyValue("text").toString();
}

void XYPadGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

double XYPadGizmo::minX() const
{
    return propertyValue("minX").toDouble();
}

void XYPadGizmo::setMinX(double t_value)
{
    setPropertyValue("minX", t_value);
}

double XYPadGizmo::maxX() const
{
    return propertyValue("maxX").toDouble();
}

void XYPadGizmo::setMaxX(double t_value)
{
    setPropertyValue("maxX", t_value);
}

double XYPadGizmo::valueX() const
{
    return propertyValue("valueX").toDouble();
}

void XYPadGizmo::setValueX(double t_value)
{
    setPropertyValue("valueX", t_value);
}

double XYPadGizmo::minY() const
{
    return propertyValue("minY").toDouble();
}

void XYPadGizmo::setMinY(double t_value)
{
    setPropertyValue("minY", t_value);
}

double XYPadGizmo::maxY() const
{
    return propertyValue("maxY").toDouble();
}

void XYPadGizmo::setMaxY(double t_value)
{
    setPropertyValue("maxY", t_value);
}

double XYPadGizmo::valueY() const
{
    return propertyValue("valueY").toDouble();
}

void XYPadGizmo::setValueY(double t_value)
{
    setPropertyValue("valueY", t_value);
}

QVector<SurfaceGizmo::GizmoOutput> XYPadGizmo::outputs() const
{
    return {
        {"valueX", "X", GizmoProperty::Number},
        {"valueY", "Y", GizmoProperty::Number}
    };
}

QVariant XYPadGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "valueX")
        return valueX();
    if(t_portId == "valueY")
        return valueY();
    return {};
}

} // namespace photon
