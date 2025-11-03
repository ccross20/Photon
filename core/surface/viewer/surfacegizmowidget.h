#ifndef SURFACEGIZMOWIDGET_H
#define SURFACEGIZMOWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceGizmoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SurfaceGizmoWidget(SurfaceGizmo *gizmo, SurfaceMode mode, QWidget *parent = nullptr);

    virtual void updateGizmo();

signals:
};

} // namespace photon

#endif // SURFACEGIZMOWIDGET_H
