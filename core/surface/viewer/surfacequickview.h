#ifndef SURFACEQUICKVIEW_H
#define SURFACEQUICKVIEW_H

#include <QQuickWidget>
#include <QVariantList>
#include "photon-global.h"

namespace photon {

// QML-hosted perform view for a Surface. Renders each gizmo via a per-type QML
// component and drives the C++ gizmo model directly (two-way). This is the
// first piece of the QML viewer; the QWidget viewer remains for edit mode.
class PHOTONCORE_EXPORT SurfaceQuickView : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantList gizmos READ gizmos NOTIFY gizmosChanged)
public:
    explicit SurfaceQuickView(QWidget *parent = nullptr);

    void setSurface(Surface *);
    Surface *surface() const;

    // Flat list of the surface's gizmos as QObject* for QML consumption.
    QVariantList gizmos() const;

signals:
    void gizmosChanged();

private:
    Surface *m_surface = nullptr;
};

} // namespace photon

#endif // SURFACEQUICKVIEW_H
