#ifndef PALETTEGIZMOEDITOR_H
#define PALETTEGIZMOEDITOR_H

#include <QWidget>
#include "surface/palettegizmo.h"

namespace photon {

class PaletteGizmoEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PaletteGizmoEditor(PaletteGizmo *t_gizmo, QWidget *parent = nullptr);

signals:
};

} // namespace photon

#endif // PALETTEGIZMOEDITOR_H
