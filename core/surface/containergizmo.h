#ifndef CONTAINERGIZMO_H
#define CONTAINERGIZMO_H

#include "surfacegizmo.h"

namespace photon {

// A gizmo that holds child gizmos and arranges them with a layout. Because it
// is itself a SurfaceGizmo, containers nest arbitrarily: this single type
// provides layout containers (Absolute/Vertical/Horizontal/Grid) and, later,
// nestable pages. The Surface is one root ContainerGizmo.
class PHOTONCORE_EXPORT ContainerGizmo : public SurfaceGizmo
{
    Q_OBJECT
    Q_PROPERTY(QVariantList childItems READ childItemsList NOTIFY childrenChanged)
public:
    const static QByteArray GizmoId;

    ContainerGizmo();
    ~ContainerGizmo();

    const QVector<SurfaceGizmo*> &children() const;

    // Collect this container's descendant gizmos into `out`. Containers are
    // included only when includeContainers is true.
    void collectDescendants(QVector<SurfaceGizmo*> &out, bool includeContainers = true) const;

    SurfaceGizmo *findDescendantWithUniqueId(const QByteArray &) const;
    // The container directly holding the given gizmo, or nullptr.
    ContainerGizmo *parentContainerOf(SurfaceGizmo *) const;

    // QML: children as QObject* for recursive rendering.
    QVariantList childItemsList() const;

    QVector<GizmoOutput> outputs() const override { return {}; }

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

public slots:
    void addChild(photon::SurfaceGizmo *, int index = -1);
    void removeChild(photon::SurfaceGizmo *);

signals:
    void childrenChanged();

private:
    QVector<SurfaceGizmo*> m_children;
};

} // namespace photon

#endif // CONTAINERGIZMO_H
