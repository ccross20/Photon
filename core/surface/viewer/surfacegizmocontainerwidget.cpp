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
#include "surface/slidergizmo.h"
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

            // Drop new gizmos in a cascading grid so they don't stack at 0,0
            // until the QML designer lets them be dragged into place.
            auto place = [container](SurfaceGizmo *gizmo){
                const int n = container->gizmos().size();
                gizmo->setPropertyValue("x", 20.0 + (n % 4) * 180.0);
                gizmo->setPropertyValue("y", 20.0 + (n / 4) * 90.0);
                container->addGizmo(gizmo);
            };

            QMenu menu;
            menu.addAction("Toggle",[place](){place(new ToggleGizmo);});
            menu.addAction("Palette",[place](){place(new PaletteGizmo);});
            menu.addAction("Slider",[place](){place(new SliderGizmo);});

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
