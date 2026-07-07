#include "togglegizmo.h"
#include "viewer/togglegizmowidget.h"

namespace photon {

const QByteArray ToggleGizmo::GizmoId = "Toggle";
const QByteArray ToggleGizmo::GizmoToggleOnId = "ToggleOn";
const QByteArray ToggleGizmo::GizmoToggleOffId = "ToggleOff";


ToggleGizmo::ToggleGizmo():SurfaceGizmo("Toggle") {
    addProperty("text",     "Label",     GizmoProperty::Text,    QString("Toggle"));
    addProperty("isRadio",  "Radio",     GizmoProperty::Boolean, false);
    addProperty("isSticky", "Sticky",    GizmoProperty::Boolean, false);
    addProperty("offColor", "Off Color", GizmoProperty::Color,   QColor(Qt::white));
    addProperty("onColor",  "On Color",  GizmoProperty::Color,   QColor(Qt::green));
}


QString ToggleGizmo::text() const
{
    return propertyValue("text").toString();
}

void ToggleGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
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

bool ToggleGizmo::isRadio() const
{
    return propertyValue("isRadio").toBool();
}

bool ToggleGizmo::isSticky() const
{
    return propertyValue("isSticky").toBool();
}

void ToggleGizmo::setIsSticky(bool t_isSticky)
{
    setPropertyValue("isSticky", t_isSticky);
}

void ToggleGizmo::setIsRadio(bool t_isRadio)
{
    setPropertyValue("isRadio", t_isRadio);
}

bool ToggleGizmo::isPressed() const
{
    return m_isPressed;
}

double ToggleGizmo::pressTime() const
{
    return m_pressTime;
}

double ToggleGizmo::timeSincePress() const
{
    return (QDateTime::currentMSecsSinceEpoch()/1000.0) - m_pressTime;
}

void ToggleGizmo::setIsPressed(bool t_value)
{
    m_isPressed = t_value;
    if(m_isPressed)
        m_pressTime = QDateTime::currentMSecsSinceEpoch()/1000.0;
}

QVector<SurfaceGizmo::GizmoOutput> ToggleGizmo::outputs() const
{
    return { {"value", "State", GizmoProperty::Boolean} };
}

QVariant ToggleGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "value")
        return isPressed();
    return {};
}

SurfaceGizmoWidget *ToggleGizmo::createWidget(SurfaceMode mode)
{
    auto widget = new ToggleGizmoWidget(this, mode);
    connect(this, &SurfaceGizmo::gizmoUpdated, widget, &SurfaceGizmoWidget::updateGizmo);
    return widget;
}

void ToggleGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceGizmo::readFromJson(t_json, t_context);
    // Sticky toggles persist their latched runtime state.
    m_isPressed = t_json.value("isPressed").toBool();
}

void ToggleGizmo::writeToJson(QJsonObject &t_json) const
{
    SurfaceGizmo::writeToJson(t_json);
    if(isSticky())
        t_json.insert("isPressed", m_isPressed);
}

} // namespace photon
