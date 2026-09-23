#include <QLayout>
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>
#include "window.h"
#include "guimanager_p.h"
#include "panel.h"
#include "photoncore.h"
#include "project/project.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"
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
    // Neither Sequence nor Project track whether they've actually changed
    // since the last save, so every library sequence still open in a tab and
    // the project itself get asked unconditionally here rather than risking
    // a silently-missed change - see the Song Library sequence-close prompt
    // in SequenceCollection::editSequence() for the per-tab equivalent of
    // this. A library sequence whose tab was already closed was already
    // asked about there (Save or Discard) and stays loaded either way per
    // SequenceCollection::panelDestroyed() - it must NOT be asked about
    // again here just because it's still sitting in memory.
    for(auto *sequence : photonApp->sequences()->sequences())
    {
        if(!sequence->isLibrarySequence())
            continue;
        if(!photonApp->sequences()->panelFor(sequence))
            continue;

        const auto choice = QMessageBox::question(this, "Save Sequence",
            QString("Save changes to \"%1\" before closing?").arg(sequence->name()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if(choice == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
        if(choice == QMessageBox::Save)
            sequence->save(sequence->filePath());
    }

    if(photonApp->project())
    {
        const auto choice = QMessageBox::question(this, "Save Project",
            "Save changes to the project before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if(choice == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
        if(choice == QMessageBox::Save)
            photonApp->project()->save();
    }

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
}

void Window::contextMenuEvent(QContextMenuEvent *)
{

}


} // namespace exo
