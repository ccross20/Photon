#include <QLayout>
#include <QCloseEvent>
#include <QSettings>
#include "window.h"
#include "guimanager_p.h"
#include "panel.h"
#include "project/project.h"
#include "third-party/advanced-docking/DockManager.h"
#include "third-party/advanced-docking/DockAreaWidget.h"

namespace photon {


Window::Window(QWidget *parent) : QMainWindow(parent)
{
    setCentralWidget(nullptr);
    layout()->setContentsMargins(0,0,0,0);
    setAttribute(Qt::WA_DeleteOnClose, true);

    //connect(exoApp,SIGNAL(aboutToQuit()),SLOT(prepareForQuit()));

}

Window::~Window()
{

}


void Window::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    //exoApp->gui()->saveLayout();

/*
    if(exoApp->project() &&exoApp->project()->isModifiedSinceSave())
    {
        core::confirm("Save Project?",{ButtonYes,ButtonNo,ButtonCancel},[this](DialogButtonType t_type){
            if(t_type == ButtonYes)
            {
                  exoApp->saveProject();
                  close();
            }
            else if(t_type == ButtonCancel)
            {
                return;
            }
            else if(t_type == ButtonNo)
            {
                exoApp->closeProject(false);
                close();
            }


        });
        event->ignore();
        return;
    }
*/

    QSettings qsettings;
    qsettings.beginGroup("window-geometry");
    qsettings.setValue("main",saveGeometry());
    qsettings.endGroup();


    event->accept();
    QMainWindow::closeEvent(event);
    emit closeWindow(this);

    auto manager = static_cast<ads::CDockManager*>(centralWidget());

    for(auto area : manager->openedDockAreas())
    {
        for(auto w : area->dockWidgets())
        {
            w->closeDockWidget();
        }
    }
    //exoApp->quitApplication();
}

void Window::contextMenuEvent(QContextMenuEvent *)
{

}


} // namespace exo
