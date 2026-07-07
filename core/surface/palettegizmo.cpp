#include "palettegizmo.h"
#include "viewer/palettegizmowidget.h"

namespace photon {
const QByteArray PaletteGizmo::GizmoId = "Palette";

PaletteGizmo::PaletteGizmo():SurfaceGizmo(GizmoId) {
    addProperty("isSticky", "Sticky", GizmoProperty::Boolean, false);
    m_palette = {Qt::red, Qt::green, Qt::blue};
}

bool PaletteGizmo::isSticky() const
{
    return propertyValue("isSticky").toBool();
}

void PaletteGizmo::setIsSticky(bool t_value)
{
    setPropertyValue("isSticky", t_value);
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

QVariantList PaletteGizmo::swatchColors() const
{
    QVariantList list;
    for(const auto &color : m_palette)
        list.append(color.name(QColor::HexArgb));
    return list;
}

void PaletteGizmo::selectSwatch(int t_index)
{
    selectColor(t_index);
    emit valuesChanged();
}

QVector<SurfaceGizmo::GizmoOutput> PaletteGizmo::outputs() const
{
    return {
        {"color", "Color", GizmoProperty::Color},
        {"index", "Index", GizmoProperty::Number}
    };
}

QVariant PaletteGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "color")
        return selectedColor();
    if(t_portId == "index")
        return selectedColorIndex();
    return {};
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

    QJsonArray colorArray;
    for(auto color : m_palette)
        colorArray.append(color.name());
    t_json.insert("colors", colorArray);

    if(isSticky())
        t_json.insert("isPressed", m_isPressed);
}

} // namespace photon
