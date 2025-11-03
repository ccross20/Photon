#include <QStandardPaths>
#include <QFileDialog>
#include <QSettings>
#include <QInputDialog>
#include <QOpenGLContext>
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
#include "sequence/sequencecollection.h"
#include "sequence/sequence.h"
#include "surface/surfacecollection.h"
#include "opengl/openglresources.h"
#include "graph/parameter/textureparameter.h"

inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

class PhotonCore::Impl
{
public:
    Impl(PhotonCore *);
    ~Impl();

    SequenceCollection *sequences;
    ResourceManager *resources;
    Settings *settings;
    PluginFactory *plugins;
    GuiManager *gui;
    Project *project = nullptr;
    Timekeeper *timekeeper;
    BusEvaluator *busEvaluator;
    Sequence *activeSequence = nullptr;
    SequencePanel *activeSequencePanel = nullptr;
    QVersionNumber version;
    QOffscreenSurface *surface = nullptr;
    OpenGLResources *openGLResources = nullptr;
    QOpenGLContext *context = nullptr;
};

PhotonCore::Impl::Impl(PhotonCore *t_core):
    sequences(new SequenceCollection),
    resources(new ResourceManager()),
    settings(new Settings(t_core)),
    plugins(new PluginFactory(t_core)),gui(new GuiManager),timekeeper(new Timekeeper),busEvaluator(new BusEvaluator)
{

}

PhotonCore::Impl::~Impl()
{

    context->makeCurrent(surface);
    openGLResources->destroy(context);
    delete openGLResources;

    delete gui;
    delete plugins;
    delete timekeeper;
    delete settings;
    delete resources;
    delete sequences;
}

PhotonCore::PhotonCore(int &argc, char **argv) : QApplication(argc, argv),
    m_impl(new Impl(this))
{
    qRegisterMetaType<TextureData>();
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

    initSurface();
    initOpenGLResources();
}

Settings *PhotonCore::settings() const
{
    return m_impl->settings;
}

SequenceCollection *PhotonCore::sequences() const
{
    return m_impl->sequences;
}

SurfaceCollection *PhotonCore::surfaces() const
{
    if(m_impl->project)
        return m_impl->project->surfaces();
    return nullptr;
}

ResourceManager *PhotonCore::resources() const
{
    return m_impl->resources;
}

void PhotonCore::initSurface()
{
    QSurfaceFormat fmt;
    //fmt.setVersion(3, 3);
    //fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    m_impl->surface = new QOffscreenSurface;
    m_impl->surface->create();
}

void PhotonCore::initOpenGLResources()
{
    m_impl->context = new QOpenGLContext;
    m_impl->context->setShareContext(QOpenGLContext::globalShareContext());
    m_impl->context->create();
    m_impl->context->makeCurrent(m_impl->surface);

    m_impl->openGLResources = new OpenGLResources;
    m_impl->openGLResources->init(m_impl->context);
}

OpenGLResources *PhotonCore::openGLResources() const
{
    return m_impl->openGLResources;
}

QOffscreenSurface *PhotonCore::surface() const
{
    return m_impl->surface;

}

void PhotonCore::loadSequence(const QString &t_path)
{
    Sequence *sequence = new Sequence;
    sequence->load(t_path);
    m_impl->sequences->addSequence(sequence);
    m_impl->sequences->editSequence(sequence);
}

void PhotonCore::newSequence()
{
    bool ok;
    QString text = QInputDialog::getText(nullptr, "New Sequence",
                                         "Name:", QLineEdit::Normal,
                                         "Untitled", &ok);
    if (ok && !text.isEmpty())
    {
        Sequence *sequence = new Sequence;
        sequence->setName(text);
        m_impl->sequences->addSequence(sequence);
        m_impl->sequences->editSequence(sequence);
    }
}

void PhotonCore::reloadLastSession()
{
    QSettings qsettings;

    qsettings.beginGroup("app");
    QString lastProject = qsettings.value("lastproject").toString();
    QString lastSequence = qsettings.value("lastsequence").toString();
    qsettings.endGroup();


    loadProject(lastProject);
    loadSequence(lastSequence);
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

    m_impl->sequences->clear();
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
void PhotonCore::editRoutine(Routine *t_routine)
{
    RoutineEditPanel *routinePanel = static_cast<RoutineEditPanel*>(m_impl->gui->createDockedPanel("photon.routine", GuiManager::CenterDockWidgetArea, true));
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
