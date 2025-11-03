#ifndef PALETTEGIZMOWIDGET_H
#define PALETTEGIZMOWIDGET_H

#include <QWidget>
#include "surfacegizmowidget.h"
#include "surface/palettegizmo.h"

class QHBoxLayout;
class QButtonGroup;

namespace photon {


class PaletteGizmoWidget : public SurfaceGizmoWidget
{

public:
    explicit PaletteGizmoWidget(PaletteGizmo *gizmo, SurfaceMode t_mode);

    void updateGizmo() override;

private:
    PaletteGizmo *m_gizmo;
    QHBoxLayout *m_layout;
    SurfaceMode m_mode;
    QButtonGroup *m_group;

signals:
};

} // namespace photon

#endif // PALETTEGIZMOWIDGET_H
