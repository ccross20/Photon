#ifndef SURFACEQUICKVIEW_H
#define SURFACEQUICKVIEW_H

#include <QQuickWidget>
#include <QVariantList>
#include "photon-global.h"

namespace photon {

// QML-hosted view for a Surface. In perform mode it renders each gizmo via a
// per-type QML component and drives the C++ model two-way. In edit mode it adds
// the free-form designer chrome (select/drag/resize/grid). Both modes share the
// same gizmo rendering; only the surrounding QML root differs.
class PHOTONCORE_EXPORT SurfaceQuickView : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(bool editMode READ editMode CONSTANT)
    Q_PROPERTY(QVariantList gizmos READ gizmos NOTIFY gizmosChanged)
public:
    explicit SurfaceQuickView(bool editMode = false, QWidget *parent = nullptr);

    bool editMode() const;

    void setSurface(Surface *);
    Surface *surface() const;

    // Flat list of the surface's gizmos as QObject* for QML consumption.
    QVariantList gizmos() const;

    // Designer actions, callable from QML.
    Q_INVOKABLE void addGizmo(const QString &type);
    Q_INVOKABLE void removeGizmo(QObject *gizmo);

signals:
    void gizmosChanged();

private:
    bool m_editMode = false;
    Surface *m_surface = nullptr;
};

} // namespace photon

#endif // SURFACEQUICKVIEW_H
