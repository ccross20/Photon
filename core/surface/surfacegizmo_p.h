#ifndef SURFACEGIZMO_P_H
#define SURFACEGIZMO_P_H

#include "surfacegizmo.h"

namespace photon
{
class SurfaceGizmo::Impl
{
public:
    Impl(SurfaceGizmo *);
    void markChanged();

    QByteArray type;
    QByteArray uniqueId;
    QString name;
    QByteArray id;
    SurfaceGizmo *facade;
};
}


#endif // SURFACEGIZMO_P_H
