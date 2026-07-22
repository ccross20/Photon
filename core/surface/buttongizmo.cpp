#include "buttongizmo.h"

namespace photon {

const QByteArray ButtonGizmo::GizmoId = "Button";


ButtonGizmo::ButtonGizmo():SurfaceGizmo("Button") {
    const QVariantHash style{{"category", "style"}};
    addProperty("text",     "Label",     GizmoProperty::Text,    QString("Button"));
    addProperty("isRadio",  "Radio",     GizmoProperty::Boolean, false);
    addProperty("isSticky", "Sticky",    GizmoProperty::Boolean, false);
    addProperty("offColor", "Off Color", GizmoProperty::Color,   QColor(Qt::white), style);
    addProperty("onColor",  "On Color",  GizmoProperty::Color,   QColor(Qt::green), style);
}


QString ButtonGizmo::text() const
{
    return propertyValue("text").toString();
}

void ButtonGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

QColor ButtonGizmo::offColor() const
{
    return propertyValue("offColor").value<QColor>();
}

void ButtonGizmo::setOffColor(const QColor &t_value)
{
    setPropertyValue("offColor", t_value);
}

QColor ButtonGizmo::onColor() const
{
    return propertyValue("onColor").value<QColor>();
}

void ButtonGizmo::setOnColor(const QColor &t_value)
{
    setPropertyValue("onColor", t_value);
}

bool ButtonGizmo::isRadio() const
{
    return propertyValue("isRadio").toBool();
}

bool ButtonGizmo::isSticky() const
{
    return propertyValue("isSticky").toBool();
}

void ButtonGizmo::setIsSticky(bool t_isSticky)
{
    setPropertyValue("isSticky", t_isSticky);
}

void ButtonGizmo::setIsRadio(bool t_isRadio)
{
    setPropertyValue("isRadio", t_isRadio);
}

bool ButtonGizmo::isPressed() const
{
    return m_isPressed;
}

double ButtonGizmo::pressTime() const
{
    return m_pressTime;
}

double ButtonGizmo::timeSincePress() const
{
    return (QDateTime::currentMSecsSinceEpoch()/1000.0) - m_pressTime;
}

void ButtonGizmo::setIsPressed(bool t_value)
{
    m_isPressed = t_value;
    if(m_isPressed)
        m_pressTime = QDateTime::currentMSecsSinceEpoch()/1000.0;
}

void ButtonGizmo::setActive(bool t_value)
{
    setIsPressed(t_value);
    emit valuesChanged();
}

bool ButtonGizmo::isActive() const
{
    return isPressed();
}

QVector<SurfaceGizmo::GizmoOutput> ButtonGizmo::outputs() const
{
    return { {"value", "State", GizmoProperty::Boolean} };
}

QVariant ButtonGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "value")
        return isPressed();
    return {};
}


void ButtonGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceGizmo::readFromJson(t_json, t_context);
    // Sticky toggles persist their latched runtime state.
    m_isPressed = t_json.value("isPressed").toBool();
}

void ButtonGizmo::writeToJson(QJsonObject &t_json) const
{
    SurfaceGizmo::writeToJson(t_json);
    if(isSticky())
        t_json.insert("isPressed", m_isPressed);
}

} // namespace photon
