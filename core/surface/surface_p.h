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
    ContainerGizmo *root = nullptr;
    QByteArray uniqueId;
    QString name;
    Surface *facade;
    SurfaceGraph *graph;
};

}

#endif // SURFACE_P_H
