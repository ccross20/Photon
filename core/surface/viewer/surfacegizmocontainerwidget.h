#ifndef SURFACEGIZMOCONTAINERWIDGET_H
#define SURFACEGIZMOCONTAINERWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceGizmoContainerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SurfaceGizmoContainerWidget(SurfaceGizmoContainer *container, SurfaceMode mode, QWidget *parent = nullptr);

signals:
};

} // namespace photon

#endif // SURFACEGIZMOCONTAINERWIDGET_H
