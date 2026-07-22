#include "togglegizmo.h"

namespace photon {

const QByteArray ToggleGizmo::GizmoId = "Toggle";

ToggleGizmo::ToggleGizmo():SurfaceGizmo("Toggle") {
    const QVariantHash style{{"category", "style"}};
    addProperty("text",      "Label",     GizmoProperty::Text,    QString("Toggle"));
    addProperty("value",     "Value",     GizmoProperty::Boolean, false);
    addProperty("orientation", "Orientation", GizmoProperty::Options, QString("Horizontal"),
                {{"options", QStringList{"Horizontal", "Vertical"}}});
    addProperty("offColor",  "Off Color", GizmoProperty::Color,   QColor(Qt::gray), style);
    addProperty("onColor",   "On Color",  GizmoProperty::Color,   QColor(Qt::green), style);
}

QString ToggleGizmo::text() const
{
    return propertyValue("text").toString();
}

void ToggleGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

bool ToggleGizmo::value() const
{
    return propertyValue("value").toBool();
}

void ToggleGizmo::setValue(bool t_value)
{
    setPropertyValue("value", t_value);
}

QColor ToggleGizmo::offColor() const
{
    return propertyValue("offColor").value<QColor>();
}

void ToggleGizmo::setOffColor(const QColor &t_value)
{
    setPropertyValue("offColor", t_value);
}

QColor ToggleGizmo::onColor() const
{
    return propertyValue("onColor").value<QColor>();
}

void ToggleGizmo::setOnColor(const QColor &t_value)
{
    setPropertyValue("onColor", t_value);
}

void ToggleGizmo::setActive(bool t_value)
{
    setValue(t_value);
}

bool ToggleGizmo::isActive() const
{
    return value();
}

QVector<SurfaceGizmo::GizmoOutput> ToggleGizmo::outputs() const
{
    return { {"value", "State", GizmoProperty::Boolean} };
}

QVariant ToggleGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "value")
        return value();
    return {};
}

} // namespace photon
