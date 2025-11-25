#include "togglegizmo.h"
#include "viewer/togglegizmowidget.h"

namespace photon {

const QByteArray ToggleGizmo::GizmoId = "Toggle";
const QByteArray ToggleGizmo::GizmoToggleOnId = "ToggleOn";
const QByteArray ToggleGizmo::GizmoToggleOffId = "ToggleOff";


ToggleGizmo::ToggleGizmo():SurfaceGizmo("Toggle") {

}


QString ToggleGizmo::text() const
{
    return m_text;
}

void ToggleGizmo::setText(const QString &t_text)
{
    m_text = t_text;
    markChanged();
}

QColor ToggleGizmo::offColor() const
{
    return m_offColor;
}

void ToggleGizmo::setOffColor(const QColor &t_value)
{
    m_offColor = t_value;
    markChanged();
}

QColor ToggleGizmo::onColor() const
{
    return m_onColor;
}

void ToggleGizmo::setOnColor(const QColor &t_value)
{
    m_onColor = t_value;
    markChanged();
}

bool ToggleGizmo::isRadio() const
{
    return m_isRadio;
}

bool ToggleGizmo::isSticky() const
{
    return m_isSticky;
}

void ToggleGizmo::setIsSticky(bool t_isSticky)
{
    m_isSticky = t_isSticky;
    markChanged();
}

void ToggleGizmo::setIsRadio(bool t_isRadio)
{
    m_isRadio = t_isRadio;
    markChanged();
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

SurfaceGizmoWidget *ToggleGizmo::createWidget(SurfaceMode mode)
{
    auto widget = new ToggleGizmoWidget(this, mode);
    connect(this, &SurfaceGizmo::gizmoUpdated, widget, &SurfaceGizmoWidget::updateGizmo);
    return widget;
}

void ToggleGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceGizmo::readFromJson(t_json, t_context);
    m_onColor = QColor::fromString(t_json.value("onColor").toString());
    m_offColor = QColor::fromString(t_json.value("offColor").toString());
    m_isSticky = t_json.value("isSticky").toBool();
    m_isPressed = t_json.value("isPressed").toBool();
    m_text = t_json.value("text").toString("Toggle");
}

void ToggleGizmo::writeToJson(QJsonObject &t_json) const
{
    SurfaceGizmo::writeToJson(t_json);
    t_json.insert("isSticky", m_isSticky);
    t_json.insert("onColor", m_onColor.name());
    t_json.insert("offColor", m_offColor.name());
    t_json.insert("text", m_text);

    if(m_isSticky)
        t_json.insert("isPressed", m_isPressed);
}

} // namespace photon
