#ifndef TOGGLEGIZMO_H
#define TOGGLEGIZMO_H
#include "surfacegizmo.h"

namespace photon {

class PHOTONCORE_EXPORT ToggleGizmo : public SurfaceGizmo
{
public:
    ToggleGizmo();

    bool isRadio() const;

    SurfaceGizmoWidget *createWidget(SurfaceMode mode) override;

};

} // namespace photon

#endif // TOGGLEGIZMO_H
