#ifndef TOGGLEGIZMOEDITOR_H
#define TOGGLEGIZMOEDITOR_H

#include <QWidget>
#include "surface/togglegizmo.h"

namespace photon {

class ToggleGizmoEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ToggleGizmoEditor(ToggleGizmo *t_gizmo, QWidget *parent = nullptr);

signals:
};

} // namespace photon

#endif // TOGGLEGIZMOEDITOR_H
