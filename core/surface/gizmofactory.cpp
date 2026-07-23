#include "gizmofactory.h"
#include "buttongizmo.h"
#include "togglegizmo.h"
#include "palettegizmo.h"
#include "slidergizmo.h"
#include "dialgizmo.h"
#include "xypadgizmo.h"
#include "huegizmo.h"
#include "segmentedgizmo.h"
#include "containergizmo.h"

namespace photon {

GizmoFactory::GizmoFactory() {}


QVector<GizmoFactory::GizmoInfo> GizmoFactory::getGizmoInfo()
{
    QVector<GizmoFactory::GizmoInfo> results;
    results << GizmoInfo("Button",ButtonGizmo::GizmoId);
    results << GizmoInfo("Toggle",ToggleGizmo::GizmoId);
    results << GizmoInfo("Dial",DialGizmo::GizmoId);
    results << GizmoInfo("XY Pad",XYPadGizmo::GizmoId);
    results << GizmoInfo("Hue",HueGizmo::GizmoId);
    results << GizmoInfo("Segmented",SegmentedGizmo::GizmoId);

    return results;
}

SurfaceGizmo *GizmoFactory::createGizmo(const QByteArray &t_id)
{
    if(t_id == ButtonGizmo::GizmoId)
        return new ButtonGizmo();
    if(t_id == ToggleGizmo::GizmoId)
        return new ToggleGizmo();
    if(t_id == PaletteGizmo::GizmoId)
        return new PaletteGizmo();
    if(t_id == SliderGizmo::GizmoId)
        return new SliderGizmo();
    if(t_id == DialGizmo::GizmoId)
        return new DialGizmo();
    if(t_id == XYPadGizmo::GizmoId)
        return new XYPadGizmo();
    if(t_id == HueGizmo::GizmoId)
        return new HueGizmo();
    if(t_id == SegmentedGizmo::GizmoId)
        return new SegmentedGizmo();
    if(t_id == ContainerGizmo::GizmoId)
        return new ContainerGizmo();


    qDebug() << "[Gizmo Factory] could not find gizmo with id: " << t_id;
    return nullptr;
}

} // namespace photon
