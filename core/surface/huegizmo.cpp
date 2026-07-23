#include <QColor>
#include "huegizmo.h"

namespace photon {

const QByteArray HueGizmo::GizmoId = "Hue";

HueGizmo::HueGizmo():SurfaceGizmo("Hue")
{
    addProperty("text", "Label", GizmoProperty::Text,   QString("Hue"));
    addProperty("hue",  "Hue",   GizmoProperty::Number, 0.0);
}

QString HueGizmo::text() const
{
    return propertyValue("text").toString();
}

void HueGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

double HueGizmo::hue() const
{
    return propertyValue("hue").toDouble();
}

void HueGizmo::setHue(double t_value)
{
    setPropertyValue("hue", t_value);
}

QVector<SurfaceGizmo::GizmoOutput> HueGizmo::outputs() const
{
    return {
        {"hue",   "Hue",   GizmoProperty::Number},
        {"color", "Color", GizmoProperty::Color}
    };
}

QVariant HueGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "hue")
        return hue();
    if(t_portId == "color")
        return QColor::fromHsvF(hue() / 360.0, 1.0, 1.0);
    return {};
}

} // namespace photon
