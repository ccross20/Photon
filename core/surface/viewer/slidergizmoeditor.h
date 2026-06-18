#ifndef SLIDERGIZMOEDITOR_H
#define SLIDERGIZMOEDITOR_H

#include <QWidget>
#include "surface/slidergizmo.h"

namespace photon {

class SliderGizmoEditor : public QWidget
{
    Q_OBJECT
public:
    explicit SliderGizmoEditor(SliderGizmo *t_gizmo, QWidget *parent = nullptr);
};

} // namespace photon

#endif // SLIDERGIZMOEDITOR_H
