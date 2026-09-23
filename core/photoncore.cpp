#include <QStandardPaths>
#include <QFileDialog>
#include <QSettings>
#include <QQuickWindow>
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
#include "rhi/rhicontext.h"
#include "graph/node/canvas/canvasrendermanager.h"
#include "graph/parameter/textureparameter.h"
#include "graph/parameter/rhitextureparameter.h"
#include "virtualdj/virtualdjconnector.h"
#include "library/songlibrary.h"
#include "settings/applicationsettings.h"
#include "fixture/fixturelibrary.h"
#include "color/colorcollection.h"
#include "color/colorresource.h"
#include "color/colorselectorwidget.h"

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
    RhiContext *rhiContext = nullptr;
    CanvasRenderManager *canvasRenderManager = nullptr;
    VirtualDJConnector *djConnector = nullptr;
    SongLibrary *songLibrary = nullptr;
    FixtureLibrary *fixtureLibrary = nullptr;
};

PhotonCore::Impl::Impl(PhotonCore *t_core):
    sequences(new SequenceCollection),
    resources(new ResourceManager()),
    settings(new Settings(t_core)),
    plugins(new PluginFactory(t_core)),gui(new GuiManager),timekeeper(new Timekeeper),busEvaluator(new BusEvaluator),djConnector(new VirtualDJConnector),
    songLibrary(new SongLibrary),
    fixtureLibrary(new FixtureLibrary)
{
}

PhotonCore::Impl::~Impl()
{
    // Tear down the GUI FIRST: any docked panel can own a keira::Scene (e.g. a
    // SequenceWidget's clip-graph editor, BusPanel, RoutineEditPanel), and each
    // Scene has its own GraphEvaluator eval thread that keeps ticking - and
    // locking mutexes inside - whatever Graph it's pointed at until the Scene
    // is destroyed. If project/sequences (and so their graphs) were freed
    // first, a still-live Scene's eval thread can fault against freed memory.
    // GuiManager::~GuiManager() synchronously deletes the whole window tree
    // (not a deferred close), so every panel - and its Scene, if any - is
    // fully gone before this returns.
    delete gui;

    // Stop the eval thread (and let go of the project's bus) before anything it
    // might reference starts getting torn down below - otherwise a tick landing
    // mid-shutdown dereferences already-freed state. Mirrors the ordering
    // PhotonCore::closeProject() uses when switching projects; unlike that path
    // there's no one left to notify, so this skips straight to the delete.
    // BusEvaluator's destructor synchronously stops/joins its eval thread before
    // returning, so the project below is guaranteed nothing can tick it anymore.
    delete busEvaluator;
    delete project;

    delete djConnector;
    delete songLibrary;   // just a QSqlDatabase connection, no ordering hazard
    delete fixtureLibrary;   // plain in-memory catalog, no ordering hazard
    delete canvasRenderManager;   // stop the render timer before the device it uses
    delete rhiContext;   // owns the process-wide graphics device

    delete plugins;
    delete timekeeper;
    delete settings;
    delete resources;
    delete sequences;
}

PhotonCore::PhotonCore(int &argc, char **argv) : QApplication(argc, argv),
    m_impl(new Impl(this))
{
    // Qt Quick backend for the surface views (the only Quick content in the
    // app). Must run before the first QQuickWindow is created.
    //
    // macOS: Metal. This used to be the software renderer, because QQuickWidget
    // on macOS's deprecated OpenGL crashed in the driver
    // (gldUpdateReadFramebuffer, inside CGLFlushDrawable) whenever its FBO was
    // recreated - which any scene-graph change triggers, e.g. adding a gizmo.
    // Metal has no such problem and keeps Quick GPU-accelerated. It also has to
    // match the widget backing store, which goes Metal on macOS as soon as a
    // QQuickWidget is composited; mixing that with a QOpenGLWidget in the same
    // top-level is unsupported, which is why none are left (see the Canvas
    // Preview panel, now QRhi-backed).
    //
    // Elsewhere: OpenGL, to match the rest of the app. On Windows Quick
    // otherwise defaults to Direct3D 11, and the graphics-API mismatch
    // forces the top-level window to be recreated the first time a
    // QQuickWidget appears, flashing the whole window.
#if defined(Q_OS_MACOS)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Metal);
#else
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif

    qRegisterMetaType<TextureData>();
    qRegisterMetaType<RhiTextureData>();
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

    // Open lazily against whatever path is currently configured; left closed
    // (SongLibrary::isOpen() == false) if none is set yet, so callers can
    // prompt the user to configure one instead of failing silently. Must run
    // after setOrganizationName()/setApplicationName() above - ApplicationSettings
    // reads a default-constructed QSettings, which resolves to the wrong location
    // until those are set, so this can't happen any earlier (e.g. in Impl's ctor).
    const QString libraryPath = ApplicationSettings::songDataLibraryPath();
    if(!libraryPath.isEmpty())
        m_impl->songLibrary->open(libraryPath);

    // Populates the "Add Fixture" picker's list; needs the app/org name set
    // above (appDataPath() resolves through them) but nothing else, so it's
    // safe this early.
    m_impl->fixtureLibrary->scan();

    // photon-ui sits below core and can't reach Project directly (see the
    // project's layering rule) - this is the one seam that lets every
    // ColorSelectorWidget with FeaturePalette set show the current project's
    // saved colours, queried live so it always reflects whichever project is
    // open (New/Load swap it out from under this lambda).
    ColorSelectorWidget::setSavedColorsProvider([](){
        QVector<QColor> saved;
        if(Project *project = photonApp->project())
            for(auto *color : project->colors()->colors())
                saved.append(color->color());
        return saved;
    });

    m_impl->resources->addResource(":/resources/styles.css", photon::Resource::ResourceStyle);

    setStyleSheet(m_impl->settings->globalStylesheet());

    m_impl->settings->init();
    m_impl->plugins->loadPluginsFromFolder(pluginDirectory());
    m_impl->plugins->init();
    m_impl->gui->init();


    // Create the offscreen QRhi device now, on the main thread — canvas nodes are
    // evaluated on the graph worker thread and must never trigger its (main-thread-
    // only) creation. The render manager then drives all canvas GPU work here.
    m_impl->canvasRenderManager = new CanvasRenderManager(rhiContext(), this);

    // Opt-in smoke test of the offscreen QRhi device (set PHOTON_RHI_SELFTEST=1).
    // Off by default so normal startup doesn't pay for a GPU render+readback; the
    // standalone rhi-spike target covers this in CI/manual runs.
    if (qEnvironmentVariableIsSet("PHOTON_RHI_SELFTEST"))
        rhiContext()->selfTest();
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

RhiContext *PhotonCore::rhiContext() const
{
    if (!m_impl->rhiContext)
        m_impl->rhiContext = new RhiContext;
    return m_impl->rhiContext;
}

void PhotonCore::loadSequence(const QString &t_path)
{
    // If this file is already open (e.g. double-clicking the same Song
    // Library sequence twice), reuse it instead of loading a second copy
    // into a duplicate tab.
    for(auto *existing : m_impl->sequences->sequences())
    {
        if(existing->filePath() == t_path)
        {
            m_impl->sequences->editSequence(existing);
            return;
        }
    }

    // Always file-backed - a Song Library sequence (the only caller of this
    // today) or an old standalone .seq being opened directly.
    Sequence *sequence = new Sequence;
    sequence->setIsLibrarySequence(true);
    sequence->load(t_path);
    m_impl->sequences->addSequence(sequence);
    m_impl->sequences->editSequence(sequence);
}

Sequence *PhotonCore::newSequence()
{
    // Created from the Project panel's "Add Sequence" - embedded directly in
    // the current project's own file (see Project::writeToJson), never a
    // separate .seq file the user has to manage.
    Sequence *sequence = new Sequence;
    sequence->setName("Untitled");
    sequence->init();
    m_impl->sequences->addSequence(sequence);
    if(m_impl->project)
        m_impl->project->sequences()->addSequence(sequence);
    m_impl->sequences->editSequence(sequence);
    return sequence;
}

void PhotonCore::reloadLastSession()
{
    QSettings qsettings;

    qsettings.beginGroup("app");
    QString lastProject = qsettings.value("lastproject").toString();
    qsettings.endGroup();

    // A project's own (internal) sequences load automatically as part of
    // loadProject() below - there's no separate "last sequence" to restore
    // any more (that QSettings key predates project-embedded sequences and
    // was never actually scoped to which project it belonged to).
    loadProject(lastProject);
}

void PhotonCore::newProject()
{
    Project *project = new Project;

    setProject(project);
}

bool PhotonCore::loadProject(const QString &path)
{
    Project *project = new Project();
    bool loaded = project->load(path);

    setProject(project);
    return loaded;
}

void PhotonCore::closeProject()
{
    if(!m_impl->project)
        return;

    m_impl->sequences->clear();
    emit projectWillClose(m_impl->project);

    // Detach the evaluator from this project's bus BEFORE deleting it, so the
    // eval thread can't tick a freed graph during the swap (setBus is now
    // synchronous — it returns only once the eval thread has let go of the bus).
    m_impl->busEvaluator->setBus(nullptr);

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

VirtualDJConnector *PhotonCore::djConnector() const
{
    return m_impl->djConnector;
}

SongLibrary *PhotonCore::songLibrary() const
{
    return m_impl->songLibrary;
}

FixtureLibrary *PhotonCore::fixtureLibrary() const
{
    return m_impl->fixtureLibrary;
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
