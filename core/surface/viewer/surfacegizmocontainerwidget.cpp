#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include "surfacegizmocontainerwidget.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/surfacegizmo.h"
#include "surfacegizmowidget.h"
#include "actioneditwidget.h"
#include "surface/togglegizmo.h"
#include "surface/palettegizmo.h"
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

    qDebug() << "Layout";

    if(mode == Mode_Edit)
    {
        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0,0,0,0);
        auto addButton = new QPushButton("Add to Container");






        connect(addButton, &QPushButton::clicked, [container, addButton](){

            QMenu menu;
            menu.addAction("Toggle",[container](){container->addGizmo(new ToggleGizmo);});
            menu.addAction("Palette",[container](){container->addGizmo(new PaletteGizmo);});

            menu.exec(addButton->mapToGlobal(addButton->rect().bottomLeft()));

        });
        hLayout->addWidget(addButton);

        QPushButton *editButton = new QPushButton("Edit Actions");
        editButton->setMaximumHeight(1000);
        hLayout->addWidget(editButton);

        connect(editButton, &QPushButton::clicked, [container](){
            ActionEditWidget *editWidget = new ActionEditWidget();
            editWidget->setContainer(container);
            photonApp->gui()->showPopoverWidget(editWidget);
        });

        vLayout->addLayout(hLayout);



    }

    setLayout(vLayout);

}

} // namespace photon
