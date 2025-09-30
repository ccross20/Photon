#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "surfacegizmocontainerwidget.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/surfacegizmo.h"
#include "surfacegizmowidget.h"
#include "surface/togglegizmo.h"
#include "surfacegraphwidget.h"
#include "photoncore.h"
#include "gui/guimanager.h"

namespace photon {

SurfaceGizmoContainerWidget::SurfaceGizmoContainerWidget(SurfaceGizmoContainer *container, SurfaceMode mode, QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *vLayout = new QVBoxLayout;

    for(const auto gizmo : container->gizmos())
    {
        vLayout->addWidget(gizmo->createWidget(mode));
    }

    if(mode == Mode_Edit)
    {
        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0,0,0,0);
        auto addButton = new QPushButton("Add to Container");

        connect(addButton, &QPushButton::clicked, [container](){
            container->addGizmo(new ToggleGizmo);
        });
        hLayout->addWidget(addButton);

        QPushButton *editButton = new QPushButton("Edit Graph");
        editButton->setMaximumHeight(1000);
        hLayout->addWidget(editButton);

        connect(editButton, &QPushButton::clicked, [container](){
            SurfaceGraphWidget *graphWidget = new SurfaceGraphWidget(container->graph());
            photonApp->gui()->showPopoverWidget(graphWidget);
        });

        vLayout->addLayout(hLayout);



    }

    setLayout(vLayout);

}

} // namespace photon
