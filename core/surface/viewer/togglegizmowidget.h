#ifndef TOGGLEGIZMOWIDGET_H
#define TOGGLEGIZMOWIDGET_H
#include "surfacegizmowidget.h"
#include "surface/togglegizmo.h"

namespace photon {

class ToggleGizmoWidget : public SurfaceGizmoWidget
{
public:
    ToggleGizmoWidget(ToggleGizmo *gizmo, SurfaceMode mode);
};

} // namespace photon

#endif // TOGGLEGIZMOWIDGET_H
