#include <QJsonArray>
#include "propertyaddress.h"

namespace photon {

const QByteArray PropertyAddress::KindProject  = "project";
const QByteArray PropertyAddress::KindGraph    = "graph";
const QByteArray PropertyAddress::KindNode     = "node";
const QByteArray PropertyAddress::KindSurface  = "surface";
const QByteArray PropertyAddress::KindGizmo    = "gizmo";
const QByteArray PropertyAddress::KindRig      = "rig";
const QByteArray PropertyAddress::KindObject   = "object";
const QByteArray PropertyAddress::KindResource = "resource";
const QByteArray PropertyAddress::KindChannelEffect = "channelEffect";
const QByteArray PropertyAddress::KindClip = "clip";

void PropertyAddress::append(const QByteArray &kind, const QByteArray &id, const QString &label)
{
    m_segments.append({ kind, id, label });
}

PropertyAddress::Segment PropertyAddress::last() const
{
    return m_segments.isEmpty() ? Segment{} : m_segments.last();
}

QString PropertyAddress::toString() const
{
    QStringList parts;
    parts.reserve(m_segments.size());
    for (const Segment &s : m_segments)
        parts.append(QString::fromUtf8(s.id.isEmpty() ? s.kind : s.id));
    return parts.join('/');
}

QString PropertyAddress::toDisplayString() const
{
    QStringList parts;
    parts.reserve(m_segments.size());
    for (const Segment &s : m_segments)
        parts.append(s.label);
    return parts.join(" / ");
}

void PropertyAddress::writeToJson(QJsonObject &t_json) const
{
    QJsonArray arr;
    for (const Segment &s : m_segments) {
        QJsonObject o;
        o.insert("kind", QString::fromUtf8(s.kind));
        o.insert("id", QString::fromUtf8(s.id));
        // The label is persisted only so a pinned tab can still be drawn if its
        // target has gone; a resolved tab relabels itself from the live object.
        o.insert("label", s.label);
        arr.append(o);
    }
    t_json.insert("segments", arr);
}

void PropertyAddress::readFromJson(const QJsonObject &t_json)
{
    m_segments.clear();
    const QJsonArray arr = t_json.value("segments").toArray();
    for (const auto &v : arr) {
        const QJsonObject o = v.toObject();
        m_segments.append({ o.value("kind").toString().toUtf8(),
                            o.value("id").toString().toUtf8(),
                            o.value("label").toString() });
    }
}

} // namespace photon
