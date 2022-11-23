#include <QStandardPaths>
#include <QFileDialog>
#include "photoncore.h"
#include "gui/guimanager.h"
#include "plugin/pluginfactory.h"
#include "project/project.h"
#include "gui/panel/routineeditpanel.h"
#include "gui/panel/sequencepanel.h"
#include "timekeeper.h"
#include "graph/bus/busevaluator.h"
#include "settings/resourcemanager.h"
#include "settings/settings.h"

inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

class PhotonCore::Impl
{
public:
    Impl(PhotonCore *);
    ~Impl();


    ResourceManager *resources;
    Settings *settings;
    PluginFactory *plugins;
    GuiManager *gui;
    Project *project = nullptr;
    Timekeeper *timekeeper;
    BusEvaluator *busEvaluator;
    SequencePanel *activeSequencePanel = nullptr;
    QVersionNumber version;
};

PhotonCore::Impl::Impl(PhotonCore *t_core):    
    resources(new ResourceManager()),
    settings(new Settings(t_core)),
    plugins(new PluginFactory(t_core)),gui(new GuiManager),timekeeper(new Timekeeper),busEvaluator(new BusEvaluator)
{

}

PhotonCore::Impl::~Impl()
{
    delete gui;
    delete plugins;
    delete timekeeper;
    delete settings;
    delete resources;
}

PhotonCore::PhotonCore(int &argc, char **argv) : QApplication(argc, argv),
    m_impl(new Impl(this))
{
    connect(m_impl->resources,&ResourceManager::resourceAdded, this, &PhotonCore::resourceAdded);
    connect(m_impl->settings, &Settings::settingsChanged, this, &PhotonCore::settingsChanged);
}

PhotonCore::~PhotonCore()
{
    delete m_impl;
}

QDir PhotonCore::pluginDirectory()
{
    QDir pluginsDir{photonApp->applicationDirPath()};

#if defined(Q_OS_WIN)
    //if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        //m_pluginsDir.cdUp();
#elif defined(Q_OS_MAC)

#endif
    pluginsDir.cd("extensions");

    return pluginsDir;
}

void PhotonCore::init()
{
    initPluginResource();
    setOrganizationName("ExothermicSystems");
    setApplicationName("Photon");
    m_impl->version = QVersionNumber(0,0,1);
    setApplicationVersion(m_impl->version.toString());


    m_impl->resources->addResource(":/resources/styles.css", photon::Resource::ResourceStyle);

    setStyleSheet(m_impl->settings->globalStylesheet());

    m_impl->settings->init();
    m_impl->plugins->loadPluginsFromFolder(pluginDirectory());
    m_impl->plugins->init();
    m_impl->gui->init();

    connect(m_impl->timekeeper, &Timekeeper::tick, m_impl->busEvaluator, &BusEvaluator::evaluate);
}

Settings *PhotonCore::settings() const
{
    return m_impl->settings;
}

ResourceManager *PhotonCore::resources() const
{
    return m_impl->resources;
}

void PhotonCore::newProject()
{
    Project *project = new Project;

    setProject(project);
}

void PhotonCore::loadProject(const QString &path)
{
    Project *project = new Project();
    project->load(path);

    setProject(project);
}

void PhotonCore::closeProject()
{
    if(!m_impl->project)
        return;

    emit projectWillClose(m_impl->project);

    delete m_impl->project;

    m_impl->project = nullptr;

    emit projectDidClose();
}

void PhotonCore::setProject(Project *t_project)
{
    if(m_impl->project == t_project)
        return;

    if(m_impl->project)
        closeProject();

    emit projectWillOpen();
    m_impl->project = t_project;
    m_impl->busEvaluator->setBus(m_impl->project->bus());
    emit projectDidOpen(m_impl->project);
}

Project *PhotonCore::project() const
{
    return m_impl->project;
}

GuiManager *PhotonCore::gui() const
{
    return m_impl->gui;
}

BusEvaluator *PhotonCore::busEvaluator() const
{
    return m_impl->busEvaluator;
}

Timekeeper *PhotonCore::timekeeper() const
{
    return m_impl->timekeeper;
}

void PhotonCore::setActiveSequencePanel(SequencePanel *panel)
{
    m_impl->activeSequencePanel = panel;
}

SequencePanel *PhotonCore::activeSequencePanel() const
{
    return m_impl->activeSequencePanel;
}

void PhotonCore::editSequence(Sequence *t_sequence)
{
    SequencePanel *sequencePanel = static_cast<SequencePanel*>(m_impl->gui->createDockedPanel("sequence", GuiManager::CenterDockWidgetArea, true));
    sequencePanel->setSequence(t_sequence);
    setActiveSequencePanel(sequencePanel);
}

void PhotonCore::editRoutine(Routine *t_routine)
{
    RoutineEditPanel *routinePanel = static_cast<RoutineEditPanel*>(m_impl->gui->createDockedPanel("routine-edit", GuiManager::CenterDockWidgetArea, true));
    routinePanel->setRoutine(t_routine);
}

PluginFactory *PhotonCore::plugins() const
{
    return m_impl->plugins;
}

QString PhotonCore::appDataPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QVersionNumber PhotonCore::version() const
{
    return m_impl->version;
}

void PhotonCore::settingsChanged()
{
    setPalette(m_impl->settings->palette());
    setStyleSheet(m_impl->settings->globalStylesheet());
}

void PhotonCore::resourceAdded(const photon::Resource &resource)
{
    bool error;
    switch(resource.type()) {
        case Resource::ResourceSetting:
            m_impl->settings->loadFromFile(resource.path());
        break;
        case Resource::ResourceStyle:
           m_impl->settings->appendStylesheet(settings()->injectSettings(resource.loadToString(), &error));
        break;
        case Resource::ResourceLocalization:
           //m_impl->localization->loadFromResource(resource);
        break;
        case Resource::ResourceIcons:
           //m_impl->iconFactory->loadFromResource(resource);
        break;
        case Resource::ResourceLayout:

            break;
        case Resource::ResourceMenu:

            break;
        }
}
} // namespace photon
