#ifndef PHOTON_PROPERTYADDRESS_H
#define PHOTON_PROPERTYADDRESS_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include "photon-global.h"

namespace photon {

// Where a property page lives, as a path from a project-level root down to the
// thing being edited - "graph/subgraph/node", "surface/element",
// "rig/object". The Properties panel renders it as clickable breadcrumbs, and
// a pinned tab stores it so the same target can be found again after reload.
//
// Addresses are values, not pointers: every segment carries the target's
// uniqueId, so an address survives the object being destroyed and recreated
// (project reload) and can be compared for equality without either end being
// alive. Resolving one back to a live object is PropertySubjectFactory's job.
class PHOTONCORE_EXPORT PropertyAddress
{
public:
    // Segment kinds. The first segment of any address is a root kind, which
    // tells the resolver which collection to start from.
    static const QByteArray KindProject;   // root: the project itself
    static const QByteArray KindGraph;     // a keira::Graph (routine / bus / subgraph)
    static const QByteArray KindNode;      // a keira::Node inside the preceding graph
    static const QByteArray KindSurface;   // a Surface
    static const QByteArray KindGizmo;     // a SurfaceGizmo inside the preceding surface
    static const QByteArray KindRig;       // root: the scene / rig tree
    static const QByteArray KindObject;    // a SceneObject
    static const QByteArray KindResource;  // any other ProjectResource
    static const QByteArray KindChannelEffect;  // a ChannelEffect on a sequence channel
    static const QByteArray KindClip;      // a Clip inside the preceding sequence

    struct Segment
    {
        QByteArray kind;
        QByteArray id;      // target's uniqueId; empty for a fixed root
        QString    label;   // what the breadcrumb shows

        bool operator==(const Segment &o) const { return kind == o.kind && id == o.id; }
    };

    PropertyAddress() = default;

    void append(const QByteArray &kind, const QByteArray &id, const QString &label);
    const QVector<Segment> &segments() const { return m_segments; }
    bool isEmpty() const { return m_segments.isEmpty(); }
    int size() const { return m_segments.size(); }

    // The leaf - what's actually being edited.
    Segment last() const;

    // Slash-joined ids, for logs and tooltips: "graph/subgraph/node".
    QString toString() const;
    // Slash-joined labels, for a compact one-line display.
    QString toDisplayString() const;

    void writeToJson(QJsonObject &) const;
    void readFromJson(const QJsonObject &);

    // Equality ignores labels - a renamed node is still the same address.
    bool operator==(const PropertyAddress &o) const { return m_segments == o.m_segments; }
    bool operator!=(const PropertyAddress &o) const { return !(*this == o); }

private:
    QVector<Segment> m_segments;
};

} // namespace photon

#endif // PHOTON_PROPERTYADDRESS_H
