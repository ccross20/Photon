#include "gizmoproperty.h"
#include <QColor>
#include <QPointF>
#include <QJsonObject>

namespace photon {

GizmoProperty::GizmoProperty(const QByteArray &t_id, const QString &t_name, Type t_type,
                             const QVariant &t_default, const QVariantHash &t_metadata)
    : m_id(t_id), m_name(t_name), m_type(t_type), m_value(t_default),
      m_default(t_default), m_metadata(t_metadata)
{
}

QVariant GizmoProperty::metadata(const QString &t_key, const QVariant &t_fallback) const
{
    return m_metadata.value(t_key, t_fallback);
}

void GizmoProperty::setMetadata(const QString &t_key, const QVariant &t_value)
{
    m_metadata.insert(t_key, t_value);
}

// Type-aware value serialization. QColor/QPointF do not round-trip through
// QJsonValue::fromVariant, so encode them explicitly.
static QJsonValue valueToJson(GizmoProperty::Type type, const QVariant &value)
{
    switch (type)
    {
    case GizmoProperty::Number:  return value.toDouble();
    case GizmoProperty::Boolean: return value.toBool();
    case GizmoProperty::Text:
    case GizmoProperty::Options: return value.toString();
    case GizmoProperty::Color:   return value.value<QColor>().name(QColor::HexArgb);
    case GizmoProperty::Point:
    {
        const QPointF p = value.toPointF();
        QJsonObject obj;
        obj.insert("x", p.x());
        obj.insert("y", p.y());
        return obj;
    }
    }
    return {};
}

static QVariant valueFromJson(GizmoProperty::Type type, const QJsonValue &json)
{
    switch (type)
    {
    case GizmoProperty::Number:  return json.toDouble();
    case GizmoProperty::Boolean: return json.toBool();
    case GizmoProperty::Text:
    case GizmoProperty::Options: return json.toString();
    case GizmoProperty::Color:   return QColor(json.toString());
    case GizmoProperty::Point:
    {
        const QJsonObject obj = json.toObject();
        return QPointF(obj.value("x").toDouble(), obj.value("y").toDouble());
    }
    }
    return {};
}

void GizmoProperty::readFromJson(const QJsonObject &t_json)
{
    if (t_json.contains("value"))
        m_value = valueFromJson(m_type, t_json.value("value"));
}

void GizmoProperty::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("id", QString(m_id));
    t_json.insert("value", valueToJson(m_type, m_value));
}

} // namespace photon
