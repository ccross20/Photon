#include "gizmofactory.h"
#include "togglegizmo.h"

namespace photon {

GizmoFactory::GizmoFactory() {}


QVector<GizmoFactory::GizmoInfo> GizmoFactory::getGizmoInfo()
{
    QVector<GizmoFactory::GizmoInfo> results;
    results << GizmoInfo("Toggle",ToggleGizmo::GizmoId);

    return results;
}

SurfaceGizmo *GizmoFactory::createGizmo(const QByteArray &t_id)
{
    if(t_id == ToggleGizmo::GizmoId)
        return new ToggleGizmo();

    qDebug() << "[Gizmo Factory] could not find gizmo with id: " << t_id;
    return nullptr;
}

} // namespace photon
