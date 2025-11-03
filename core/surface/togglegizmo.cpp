#include "togglegizmo.h"
#include "viewer/togglegizmowidget.h"

namespace photon {

const QByteArray ToggleGizmo::GizmoId = "Toggle";

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

} // namespace photon
