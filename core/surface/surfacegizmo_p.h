#ifndef SURFACEGIZMO_P_H
#define SURFACEGIZMO_P_H

#include <QHash>
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
    SurfaceGizmo *facade;
    QVector<SurfaceGizmo::GizmoHistoryEvent> histroy;
    QVector<GizmoProperty*> properties;
    QHash<QByteArray, GizmoProperty*> propertyById;
};
}


#endif // SURFACEGIZMO_P_H
