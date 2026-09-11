#include "segmentedgizmo.h"

namespace photon {

const QByteArray SegmentedGizmo::GizmoId = "Segmented";

namespace {

// Split a comma-separated property string into trimmed, non-empty entries.
QStringList splitCsv(const QString &t_csv)
{
    QStringList result;
    const auto parts = t_csv.split(',', Qt::SkipEmptyParts);
    for(const auto &part : parts)
    {
        const QString trimmed = part.trimmed();
        if(!trimmed.isEmpty())
            result.append(trimmed);
    }
    return result;
}

// A values-list entry emitted from the "value" output: numeric if it parses
// cleanly as one (so "0,1,2" behaves the same as an empty list), else the
// raw string.
QVariant parseValueEntry(const QString &t_entry)
{
    bool ok = false;
    const int asInt = t_entry.toInt(&ok);
    if(ok)
        return asInt;
    const double asDouble = t_entry.toDouble(&ok);
    if(ok)
        return asDouble;
    return t_entry;
}

} // namespace

SegmentedGizmo::SegmentedGizmo():SurfaceGizmo("Segmented")
{
    addProperty("text",          "Label",       GizmoProperty::Text,    QString("Segmented"));
    addProperty("options",       "Options",     GizmoProperty::Text,    QString("One,Two,Three"));
    // Empty by default: the "value" output then falls back to the selected
    // index (0, 1, 2, ...). Set a comma-separated list to emit those instead.
    addProperty("values",        "Values",      GizmoProperty::Text,    QString(""));
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
    return splitCsv(optionsText());
}

QString SegmentedGizmo::valuesText() const
{
    return propertyValue("values").toString();
}

void SegmentedGizmo::setValuesText(const QString &t_value)
{
    setPropertyValue("values", t_value);
}

QStringList SegmentedGizmo::valueList() const
{
    return splitCsv(valuesText());
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
        const auto values = valueList();
        const int idx = selectedIndex();
        if(!values.isEmpty() && idx >= 0 && idx < values.size())
            return parseValueEntry(values[idx]);
        // No values list (or a selection past its end): emit the index.
        return idx;
    }
    return {};
}

} // namespace photon
