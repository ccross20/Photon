#ifndef PHOTON_GIZMOPROPERTY_H
#define PHOTON_GIZMOPROPERTY_H

#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include "photon-global.h"

namespace photon {

// A single data-driven property of a SurfaceGizmo. Properties describe the
// gizmo's persisted, editable configuration (label, range, colors, ...) in a
// generic way so the inspector, QML bindings and serialization can all be
// generated from one definition rather than hand-written per gizmo type.
class PHOTONCORE_EXPORT GizmoProperty
{
public:
    enum Type
    {
        Number,
        Boolean,
        Text,
        Color,
        Point,
        Options
    };

    GizmoProperty() = default;
    GizmoProperty(const QByteArray &id, const QString &name, Type type,
                  const QVariant &defaultValue, const QVariantHash &metadata = {});

    QByteArray id() const { return m_id; }
    QString name() const { return m_name; }
    Type type() const { return m_type; }

    QVariant value() const { return m_value; }
    void setValue(const QVariant &value) { m_value = value; }

    QVariant defaultValue() const { return m_default; }

    // Free-form metadata (min/max/step/options/...) consumed by editors.
    const QVariantHash &metadata() const { return m_metadata; }
    QVariant metadata(const QString &key, const QVariant &fallback = {}) const;
    void setMetadata(const QString &key, const QVariant &value);

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

private:
    QByteArray m_id;
    QString m_name;
    Type m_type = Number;
    QVariant m_value;
    QVariant m_default;
    QVariantHash m_metadata;
};

} // namespace photon

#endif // PHOTON_GIZMOPROPERTY_H
