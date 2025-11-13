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

int PaletteGizmo::selectedColorIndex() const
{
    return m_selectedIndex;
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
    markChanged();
}

SurfaceGizmoWidget *PaletteGizmo::createWidget(SurfaceMode mode)
{
    auto widget = new PaletteGizmoWidget(this, mode);
    connect(this, &SurfaceGizmo::gizmoUpdated, widget, &SurfaceGizmoWidget::updateGizmo);
    return widget;
}

void PaletteGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceGizmo::readFromJson(t_json, t_context);
    m_isSticky = t_json.value("isSticky").toBool();
    m_isPressed = t_json.value("isPressed").toBool();

    m_palette.clear();
    auto colorArray = t_json.value("colors").toArray();
    for(auto value : colorArray)
    {
        m_palette.append(QColor::fromString(value.toString()));
    }
}

void PaletteGizmo::writeToJson(QJsonObject &t_json) const
{
    SurfaceGizmo::writeToJson(t_json);
    t_json.insert("isSticky", m_isSticky);

    QJsonArray colorArray;
    for(auto color : m_palette)
        colorArray.append(color.name());
    t_json.insert("colors", colorArray);

    if(m_isSticky)
        t_json.insert("isPressed", m_isPressed);
}

} // namespace photon
