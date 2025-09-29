#include "togglegizmo.h"
#include "viewer/togglegizmowidget.h"

namespace photon {

ToggleGizmo::ToggleGizmo():SurfaceGizmo("Toggle") {

}


bool ToggleGizmo::isRadio() const
{
    return false;
}

SurfaceGizmoWidget *ToggleGizmo::createWidget(SurfaceMode mode)
{
    return new ToggleGizmoWidget(this, mode);
}

} // namespace photon
