#ifndef TOGGLEGIZMOWIDGET_H
#define TOGGLEGIZMOWIDGET_H
#include "surfacegizmowidget.h"
#include "surface/togglegizmo.h"

class QPushButton;

namespace photon {

class ToggleGizmoWidget : public SurfaceGizmoWidget
{
public:
    ToggleGizmoWidget(ToggleGizmo *gizmo, SurfaceMode mode);

    void updateGizmo() override;

private:
    QPushButton *m_button;
    ToggleGizmo *m_gizmo;
};

} // namespace photon

#endif // TOGGLEGIZMOWIDGET_H
