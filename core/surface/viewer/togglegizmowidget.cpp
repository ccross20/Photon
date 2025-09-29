#include <QHBoxLayout>
#include <QPushButton>
#include "togglegizmowidget.h"

namespace photon {

ToggleGizmoWidget::ToggleGizmoWidget(ToggleGizmo *t_gizmo, SurfaceMode t_mode):SurfaceGizmoWidget(t_gizmo, t_mode)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    QPushButton *toggleButton = new QPushButton("Toggle");

    toggleButton->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
    toggleButton->setMaximumHeight(1000);
    hLayout->addWidget(toggleButton);

    if(t_mode == Mode_Edit)
    {
        QPushButton *editButton = new QPushButton("Edit");
        editButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::MinimumExpanding));
        editButton->setMaximumHeight(1000);
        hLayout->addWidget(editButton);
    }



    setLayout(hLayout);
}

} // namespace photon
