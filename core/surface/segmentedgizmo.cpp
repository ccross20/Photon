#include "segmentedgizmo.h"

namespace photon {

const QByteArray SegmentedGizmo::GizmoId = "Segmented";

SegmentedGizmo::SegmentedGizmo():SurfaceGizmo("Segmented")
{
    addProperty("text",          "Label",       GizmoProperty::Text,    QString("Segmented"));
    addProperty("options",       "Options",     GizmoProperty::Text,    QString("One,Two,Three"));
    addProperty("selectedIndex", "Selected",    GizmoProperty::Number,  0.0);
    addProperty("orientation",   "Orientation", GizmoProperty::Options, QString("Horizontal"),
                {{"options", QStringList{"Horizontal", "Vertical"}}});
}

QString SegmentedGizmo::text() const
{
    return propertyValue("text").toString();
}

void SegmentedGizmo::setText(const QString &t_text)
{
    setPropertyValue("text", t_text);
}

QString SegmentedGizmo::optionsText() const
{
    return propertyValue("options").toString();
}

void SegmentedGizmo::setOptionsText(const QString &t_value)
{
    setPropertyValue("options", t_value);
}

QStringList SegmentedGizmo::optionList() const
{
    QStringList result;
    const auto parts = optionsText().split(',', Qt::SkipEmptyParts);
    for(const auto &part : parts)
    {
        const QString trimmed = part.trimmed();
        if(!trimmed.isEmpty())
            result.append(trimmed);
    }
    return result;
}

int SegmentedGizmo::selectedIndex() const
{
    return propertyValue("selectedIndex").toInt();
}

void SegmentedGizmo::setSelectedIndex(int t_index)
{
    setPropertyValue("selectedIndex", t_index);
}

QVector<SurfaceGizmo::GizmoOutput> SegmentedGizmo::outputs() const
{
    return {
        {"selectedIndex", "Index", GizmoProperty::Number},
        {"value",         "Value", GizmoProperty::Text}
    };
}

QVariant SegmentedGizmo::outputValue(const QByteArray &t_portId) const
{
    if(t_portId == "selectedIndex")
        return selectedIndex();
    if(t_portId == "value")
    {
        const auto options = optionList();
        const int idx = selectedIndex();
        if(idx >= 0 && idx < options.size())
            return options[idx];
        return QString();
    }
    return {};
}

} // namespace photon
