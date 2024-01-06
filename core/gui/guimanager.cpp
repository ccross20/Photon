#include <QLayout>
#include <QMenuBar>
#include <QCursor>
#include <QScreen>
#include <QSettings>
#include <QKeyEvent>
#include <QMainWindow>
#include <QToolBar>
#include "guimanager_p.h"
#include "project/project.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "sequence/sequencecollection.h"
#include "panel_p.h"
/*
#include "settings/settings.h"
#include "menu/menufactory.h"
#include "menu/toolbar.h"
#include "file/scratchmanager.h"
#include "project/project.h"
#include "library/library.h"
#include "menu/contexttoolbar.h"
#include "auth/activator.h"
#include "network/updatechecker.h"
#include "gui/window.h"
*/

namespace photon {


GuiManager::Impl::Impl(GuiManager *manager) :
    m_ext(manager),
    m_activePanel(nullptr),
    m_macMenubar(nullptr)
{
    m_dockManager = new ads::CDockManager();
    m_dockManager->setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    m_dockManager->setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);

    connect(m_dockManager,SIGNAL(dockWidgetRemoved(ads::CDockWidget*)), this, SLOT(dockWidgetRemoved(ads::CDockWidget*)));
    connect(m_dockManager,SIGNAL(dockWidgetAboutToBeRemoved(ads::CDockWidget*)), this, SLOT(dockWidgetAboutToBeRemoved(ads::CDockWidget*)));
    connect(m_dockManager,SIGNAL(floatingWidgetCreated(ads::CFloatingDockContainer*)), this, SLOT(floatingWidgetCreated(ads::CFloatingDockContainer*)));
}

GuiManager::Impl::~Impl()
{

}

void GuiManager::Impl::createAppWindow()
{
    window = new Window();
    //window->setAttribute(Qt::WA_DeleteOnClose);

    QMenuBar *menubar = new QMenuBar;
    window->setMenuBar(menubar);

    QMenu *fileMenu = new QMenu("File");
    fileMenu->addAction("Quit");
    menubar->addMenu(fileMenu);

    QMenu *windowMenu = new QMenu("Window");
    windowMenu->addAction("Canvas", [](){photonApp->gui()->createFloatingPanel("photon.canvas-collection");});
    windowMenu->addAction("Pixel Layouts", [](){photonApp->gui()->createFloatingPanel("photon.pixellayout-collection");});
    windowMenu->addAction("Fixture", [](){photonApp->gui()->createFloatingPanel("photon.fixture-collection");});
    windowMenu->addAction("Routine", [](){photonApp->gui()->createFloatingPanel("photon.routine-collection");});
    windowMenu->addAction("Sequence", [](){photonApp->gui()->createFloatingPanel("photon.sequence-collection");});
    windowMenu->addAction("Canvas Viewer", [](){photonApp->gui()->createFloatingPanel("photon.canvas-viewer");});
    windowMenu->addAction("Tags", [](){photonApp->gui()->createFloatingPanel("photon.tag-collection");});
    menubar->addMenu(windowMenu);

    Panel *panel1 = createPanel("photon.bus");
    addPanel(panel1);

    ads::CDockWidget* DockWidget = new ads::CDockWidget("Bus");
    DockWidget->setObjectName("photon.bus");
    DockWidget->setWidget(panel1);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);




    QToolBar *toolBar = new QToolBar("root");
    toolBar->setObjectName("deco.core.toolbars.root");
    toolBar->setOrientation(Qt::Horizontal);
    //menus->populateToolBar(toolBar, toolBar->id());

    toolBar->addAction("Open DMX Viewer",[](){photonApp->gui()->createFloatingPanel("photon.dmx-viewer");});
    toolBar->addAction("Open Visualizer",[](){photonApp->gui()->createFloatingPanel("visualizer");});
    toolBar->addAction("Save",[](){photonApp->project()->save();});
    toolBar->addAction("Load",[](){photonApp->loadProject();});
    toolBar->addAction("New",[](){photonApp->newProject();});
    toolBar->addAction("Save Layout",[this](){m_ext->saveLayout();});
    toolBar->addAction("New Sequence",[](){photonApp->newSequence();});
    toolBar->addAction("Load Sequence",[](){photonApp->loadSequence();});
    toolBar->addAction("Save Sequence",[](){photonApp->sequences()->activeSequence()->save();});



    window->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);
    //window->addToolBar(Qt::ToolBarArea::TopToolBarArea, contextToolbar);

    centralArea = m_dockManager->setCentralWidget(DockWidget);

    window->setCentralWidget(m_dockManager);


/*
    m_ext->createDockedPanel("photon.routine-collection", RightDockWidgetArea);
    m_ext->createDockedPanel("photon.sequence-collection", RightDockWidgetArea);
    m_ext->createDockedPanel("photon.fixture-collection", RightDockWidgetArea);
    */


    QSettings qsettings;
    qsettings.beginGroup("window-geometry");


    if(qsettings.contains("main"))
        window->restoreGeometry(qsettings.value("main").toByteArray());
    else
    {
        //auto screenSize = exoApp->primaryScreen()->size();

        auto screenSize = QSize(1200,800);
        int w = screenSize.width() * .8;
        int h = screenSize.height() * .8;
        window->resize(w, h);
        window->move(screenSize.width() *1.1, screenSize.height() *.1);
    }


    qsettings.endGroup();


    #if defined(Q_OS_WIN)
    //menus->populateMenuBar(window->menuBar(), "exo.core.menus.root");
    #elif defined(Q_OS_MAC)

    m_macMenubar = new QMenuBar(win);
    m_macMenubar->setNativeMenuBar(true);

    QAction *updateAction = m_macMenubar->addAction("Update",[](){
        exoApp->updateChecker()->update();
    });

    updateAction->setMenuRole(QAction::ApplicationSpecificRole);

    menus->populateMenuBar(m_macMenubar, "deco.core.menus.root");




    #endif

}


void GuiManager::Impl::dockWidgetRemoved(ads::CDockWidget* DockWidget)
{
    //qDebug() << "Removed" << DockWidget->objectName();
}

void GuiManager::Impl::dockWidgetAboutToBeRemoved(ads::CDockWidget* DockWidget)
{
    //qDebug() << "about to Remove" << DockWidget->objectName();
}

void GuiManager::Impl::floatingWidgetCreated(ads::CFloatingDockContainer* FloatingWidget)
{
    //qDebug() << "add window" << FloatingWidget->objectName();
    FloatingWidget->setAttribute(Qt::WA_DeleteOnClose);
}



void GuiManager::Impl::launchInterface()
{
    createAppWindow();

    m_ext->restoreLayout();

    window->show();
    /*
    auto screenSize = exoApp->primaryScreen()->size();
    int w = screenSize.width() * .9;
    int h = screenSize.height() * .9;
    win->resize(w, h);

    QPoint centerPt = exoApp->primaryScreen()->geometry().center();

    win->setGeometry(centerPt.x() - (w/2), centerPt.y() - (h/2), w, h);

    win->show();
    */
    updateMainWindowTitle();


}

void GuiManager::Impl::setMainWindowTitle(const QString &title)
{
    mainWindowTitle = title;
    updateMainWindowTitle();
}

void GuiManager::Impl::setProjectModified(bool value)
{
    if(projectIsModified == value)
        return;
    projectIsModified = value;
    updateMainWindowTitle();
}

void GuiManager::Impl::updateMainWindowTitle()
{
    QString suffix = "";
/*
    if(projectIsModified)
        windows[0]->setWindowTitle(mainWindowTitle + "*" + suffix);
    else
        windows[0]->setWindowTitle(mainWindowTitle + suffix);
        */
}

Panel *GuiManager::Impl::createPanel(const PanelId &panelId)
{
    return photonApp->plugins()->createPanel(panelId);

}

void GuiManager::Impl::addPanel(Panel *panel)
{
    if(panels.contains(panel))
        return;
    connect(panel,SIGNAL(destroyed(QObject*)),m_ext,SLOT(panelDestroyed(QObject*)));
    panels.append(panel);
    panel->setGui(m_ext);

    if(photonApp->project())
        panel->projectDidOpen(photonApp->project());

    //menus->populateMenuBar(panel->menubar(), panel->id(), panel->commands());
    //menus->populateToolBar(panel->toolbar(), panel->id()+".toolbar", panel->commands());
    panel->init();
    panel->buildMenu(panel->menubar());
}

void GuiManager::Impl::reloadPanel(Panel *panel)
{
    panel->menubar()->clear();
    //menus->populateMenuBar(panel->menubar(), panel->id(), panel->commands());
    panel->buildMenu(panel->menubar());
}

void GuiManager::Impl::activatePanel(Panel *panel)
{
    if(m_activePanel)
    {
        if(m_activePanel == panel)
            return;
        m_activePanel->deactivate();
    }
    m_activePanel = panel;
    if(m_activePanel)
    {
        m_activePanel->activate();



/*
        if(m_activePanel->panelWidget())
            m_activePanel->panelWidget()->setFocus(Qt::FocusReason::MouseFocusReason);
            */
    }
}

QMenuBar *GuiManager::Impl::menubar() const
{
    /*
#if defined(Q_OS_WIN)
    return windows[0]->menuBar();
#elif defined(Q_OS_MAC)
    //return windows[0]->menuBar();
    return m_macMenubar;
#endif
*/
    return new QMenuBar;
}

GuiManager::GuiManager(QObject *parent) : QObject (parent), m_impl(new GuiManager::Impl(this))
{

}

GuiManager::~GuiManager()
{
}

void GuiManager::init()
{

    connect(photonApp,&PhotonCore::projectDidOpen,this,&GuiManager::projectDidOpen);
    connect(photonApp,&PhotonCore::projectWillClose,this,&GuiManager::projectWillClose);
    connect(photonApp,&PhotonCore::projectDidClose,this,&GuiManager::projectDidClose);
/*
    connect(exoApp->activator(),SIGNAL(didActivate()),this,SLOT(activationChanged()));
    connect(exoApp->activator(),SIGNAL(didDeactivate()),this,SLOT(activationChanged()));
    connect(exoApp,SIGNAL(aboutToQuit()),SLOT(prepareForQuit()));
    */
}

void GuiManager::panelDestroyed(QObject *obj)
{
    m_impl->panels.removeAll(static_cast<Panel *>(obj));

    if(static_cast<Panel *>(obj)->isActivated() || m_impl->activePanel() == static_cast<Panel *>(obj))
        m_impl->clearActivePanel();
}

void GuiManager::activationChanged()
{
    m_impl->updateMainWindowTitle();
}

void GuiManager::prepareForQuit()
{
    //if(core::applicationSetting("deco.settings.gui.save_on_exit",true).toBool())
        //exoApp->gui()->saveLayout(exoApp->appDataPath() + "/startup.layout");
}

void GuiManager::projectDidOpen(Project* project)
{
    //m_impl->setMainWindowTitle("Photon (" + exoApp->version().toString() + ") - " + project->name());
    m_impl->setMainWindowTitle("Photon");

    for(auto panel : m_impl->panels)
    {
        panel->projectDidOpen(project);
    }
    //m_impl->setProjectModified(project->isModifiedSinceSave());

}

void GuiManager::projectWillClose(Project* project)
{
    for(auto panel : m_impl->panels)
    {
        panel->projectWillClose(project);
    }

}

void GuiManager::projectDidClose()
{
    //m_impl->setMainWindowTitle("Exothermic (" + exoApp->version().toString() + ")");
    m_impl->setProjectModified(false);

}

void GuiManager::launchInterface()
{
    //if(!restoreLayout(exoApp->appDataPath() + "/startup.layout"))
    {
        m_impl->launchInterface();
    }


}

void GuiManager::destroyInterface()
{

    m_impl->closingInterface = true;

    m_impl->panels.clear();

    m_impl->closingInterface = false;
}

Panel *GuiManager::createPanel(const PanelId &panelId)
{
    return m_impl->createPanel(panelId);
}

Panel *GuiManager::createFloatingPanel(const PanelId &panelId)
{
    if(panelId.isEmpty())
        return nullptr;
    auto panel = createPanel(panelId);

    if(!panel)
        return nullptr;
    m_impl->addPanel(panel);
    ads::CDockWidget* DockWidget = new ads::CDockWidget(panel->id());
    DockWidget->setObjectName(panel->id());
    DockWidget->setWindowTitle(panel->name());
    DockWidget->setWidget(panel);
    DockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContent);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetAlwaysCloseAndDelete, true);
    panel->m_impl->dockWidget = DockWidget;

    connect(panel, &Panel::nameUpdated, DockWidget, [panel, DockWidget](){ DockWidget->setWindowTitle(panel->name());});

    m_impl->dockManager()->addDockWidgetFloating(DockWidget);
    return panel;
}

void GuiManager::bringPanelToFront(Panel *panel) const
{
    //qDebug() << m_impl->dockManager()->findDockWidget(panel->id())->isCurrentTab();
    if(panel->dockWidget())
        panel->dockWidget()->raise();
}

Panel *GuiManager::createDockedPanel(const PanelId &panelId, DockWidgetArea t_area, bool t_isTab)
{
    if(panelId.isEmpty())
        return nullptr;
    auto panel = createPanel(panelId);

    if(!panel)
        return nullptr;
    m_impl->addPanel(panel);
    ads::CDockWidget* DockWidget = new ads::CDockWidget(panel->id());
    DockWidget->setObjectName(panel->id());
    DockWidget->setWindowTitle(panel->name());
    DockWidget->setWidget(panel);
    DockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContent);
    DockWidget->setFeature(ads::CDockWidget::DockWidgetAlwaysCloseAndDelete, true);
    panel->m_impl->dockWidget = DockWidget;

    connect(panel, &Panel::nameUpdated, DockWidget, [panel, DockWidget](){ DockWidget->setWindowTitle(panel->name());});

    auto busDock = m_impl->dockManager()->findDockWidget("photon.bus");


    if(t_isTab)
        m_impl->dockManager()->addDockWidgetTabToArea(DockWidget,busDock ? busDock->dockAreaWidget() : nullptr);
    else
        m_impl->dockManager()->addDockWidget(static_cast<ads::DockWidgetArea>(t_area), DockWidget);
    return panel;
}

void GuiManager::insertPanel(const PanelId &panelId, uint windowIndex)
{
    Panel *newPanel = createPanel(panelId);

    if(newPanel)
    {
        insertPanel(newPanel, windowIndex);
    }
}

void GuiManager::insertPanel(Panel *panel, uint windowIndex)
{


    m_impl->addPanel(panel);
}

Panel *GuiManager::findPanel(const PanelId &panelId) const
{
    for(auto panel : m_impl->panels)
    {
        if(panel->id() == panelId)
            return panel;
    }
    return nullptr;
}

int GuiManager::panelCount() const
{
    return m_impl->panels.length();
}

Panel *GuiManager::activePanel() const
{
    return m_impl->activePanel();
}

QMenuBar *GuiManager::menubar() const
{
    return m_impl->menubar();
}

void GuiManager::clearLayout(QLayout *layout)
{
    while (layout->count()>0) {
       auto item = layout->takeAt(0);
       delete item->widget();
       if(item->layout())
          clearLayout(item->layout());
    }
}

void GuiManager::showPopoverWidget(QWidget *widget)
{
    if(m_impl->popOverWidget)
    {
        disconnect(m_impl->popOverWidget,SIGNAL(destroyed()),this,SLOT(popoverWasClosed()));
        m_impl->popOverWidget->close();
    }

    m_impl->popOverWidget = widget;
    //m_impl->popOverWidget->setParent(m_impl->windows.first());
    connect(widget,SIGNAL(destroyed()),this,SLOT(popoverWasClosed()));
    m_impl->popOverWidget->show();
}

void GuiManager::clearPopoverWidget()
{
    if(m_impl->popOverWidget)
    {
        m_impl->popOverWidget->close();
        m_impl->popOverWidget = nullptr;
    }
}

void GuiManager::popoverWasClosed()
{
    m_impl->popOverWidget = nullptr;
}

void GuiManager::setIsLocked(bool value)
{

}

bool GuiManager::isLocked() const
{
    return false;
}

void GuiManager::setIsObscured(bool value)
{

}

bool GuiManager::isObscured() const
{
    return false;
}

void GuiManager::obscureAllExcept(const QByteArrayList &panelIds)
{

}

bool GuiManager::saveLayout(const QString &t_filename)
{
    /*
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Could not open layout file for saving.";
        return false;
    }

    file.close();
    */
    QString filename = t_filename;

    if(filename.isEmpty())
        filename = photonApp->appDataPath() + "/startup.layout";

    QSettings Settings(filename, QSettings::IniFormat);

    QByteArrayList ids;
    for(auto panel : m_impl->panels)
    {
        if(panel->id() != "deco.panel.viewport")
            ids.append(panel->id());
    }
    Settings.setValue("mainWindow/ids", ids.join(","));

    //Settings.setValue("mainWindow/Geometry", m_impl->window->saveGeometry());
    Settings.setValue("mainWindow/State",  m_impl->window->saveState());
    Settings.setValue("mainWindow/DockingState", m_impl->dockManager()->saveState());

    return true;
}

bool GuiManager::restoreLayout(const QString &t_filename)
{
    /*
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open layout file for reading.";
        return false;
    }

    file.close();
    */
    QString filename = t_filename;

    if(filename.isEmpty())
        filename = photonApp->appDataPath() + "/startup.layout";

    QSettings Settings(filename, QSettings::IniFormat);

    auto ids = Settings.value("mainWindow/ids").toByteArray().split(',');

    for(auto id : ids)
    {
        createDockedPanel(id);
    }

    //m_impl->window->restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
    m_impl->window->restoreState(Settings.value("mainWindow/State").toByteArray());
    m_impl->dockManager()->restoreState(Settings.value("mainWindow/DockingState").toByteArray());

    //m_impl->dockManager()->setCentralWidget(nullptr);

    return true;
}

} // namespace exo
