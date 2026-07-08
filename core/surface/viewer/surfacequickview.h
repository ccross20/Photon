#ifndef SURFACEQUICKVIEW_H
#define SURFACEQUICKVIEW_H

#include <QQuickWidget>
#include "photon-global.h"

namespace photon {

// QML-hosted view for a Surface. Renders the surface's root ContainerGizmo
// recursively (containers arrange children by layout, nesting arbitrarily). In
// edit mode it adds the free-form designer chrome; in perform mode controls
// receive input directly.
class PHOTONCORE_EXPORT SurfaceQuickView : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(bool editMode READ editMode CONSTANT)
    Q_PROPERTY(QObject *root READ rootObject NOTIFY rootChanged)
public:
    explicit SurfaceQuickView(bool editMode = false, QWidget *parent = nullptr);

    bool editMode() const;

    void setSurface(Surface *);
    Surface *surface() const;

    // The surface's root ContainerGizmo, for QML rendering.
    QObject *rootObject() const;

    // Designer actions, callable from QML. `target` is the container to add
    // into (a ContainerGizmo); null adds to the root.
    Q_INVOKABLE void addGizmo(const QString &type, QObject *target = nullptr);
    Q_INVOKABLE void removeGizmo(QObject *gizmo);
    // Move a gizmo into `target` container at `index` (reparent), or reorder
    // within its current container when target is already its parent.
    Q_INVOKABLE void reparentGizmo(QObject *gizmo, QObject *target, int index);

signals:
    void rootChanged();

private:
    bool m_editMode = false;
    Surface *m_surface = nullptr;
};

} // namespace photon

#endif // SURFACEQUICKVIEW_H
