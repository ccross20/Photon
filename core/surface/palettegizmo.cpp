#include "palettegizmo.h"
#include "viewer/palettegizmowidget.h"

namespace photon {
const QByteArray PaletteGizmo::GizmoId = "Palette";

PaletteGizmo::PaletteGizmo():SurfaceGizmo(GizmoId) {
    m_palette = {Qt::red, Qt::green, Qt::blue};
}

bool PaletteGizmo::isSticky() const
{
    return m_isSticky;
}

void PaletteGizmo::setIsSticky(bool t_value)
{
    m_isSticky = t_value;
    markChanged();
}

bool PaletteGizmo::isPressed() const
{
    return m_isPressed;
}

void PaletteGizmo::setIsPressed(bool t_value)
{
    m_isPressed = t_value;
}

bool PaletteGizmo::hasSelectedColor() const
{
    return m_selectedIndex >= 0 && m_selectedIndex < m_palette.length();
}

QColor PaletteGizmo::selectedColor() const
{
    if(hasSelectedColor())
        return m_palette[m_selectedIndex];
    return Qt::black;
}

void PaletteGizmo::setPalette(const ColorPalette &t_palette)
{
    m_palette = t_palette;
    markChanged();
}

void PaletteGizmo::selectColor(int t_index)
{
    m_selectedIndex = t_index;
}

SurfaceGizmoWidget *PaletteGizmo::createWidget(SurfaceMode mode)
{
    auto widget = new PaletteGizmoWidget(this, mode);
    connect(this, &SurfaceGizmo::gizmoUpdated, widget, &SurfaceGizmoWidget::updateGizmo);
    return widget;
}

} // namespace photon
