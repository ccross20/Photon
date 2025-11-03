#ifndef SURFACE_P_H
#define SURFACE_P_H

#include "surface.h"

namespace photon
{

class Surface::Impl
{
public:
    Impl(Surface *);
    QVector<SurfaceGizmoContainer*> gizmos;
    QByteArray uniqueId;
    QString name;
    Surface *facade;
};

}

#endif // SURFACE_P_H
