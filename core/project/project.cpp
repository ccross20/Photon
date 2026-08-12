#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QWidget>
#include "project.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "sequence/sequencecollection.h"
#include "routine/routinecollection.h"
#include "routine/routine.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "sequence/sequence.h"
#include "graph/bus/busgraph.h"
#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/sequencenode.h"
#include "graph/bus/surfacenode.h"
#include "graph/bus/dmxwriternode.h"
#include "graph/bus/dmxsubgraphnode.h"
#include "graph/bus/identifyfixturenode.h"
#include "scene/sceneobject.h"
#include "scene/scenemanager.h"
#include "surface/surfacecollection.h"
#include "surface/surface.h"
#include "fixture/fixturegroup.h"
#include "scene/sceneiterator.h"

namespace photon {

class Project::Impl
{
public:
    Impl();
    ~Impl();

    // Give any SurfaceNode that arrived without a surface reference one of its
    // own. Covers nodes dragged in from the palette; deserialized nodes already
    // carry an id, and Graph::readFromJson doesn't emit nodeWasAdded anyway.
    void adoptSurfaceNode(keira::Node *);
    void watchBus();

    PixelLayoutCollection pixelLayouts;
    FixtureCollection fixtures;
    RoutineCollection routines;
    SurfaceCollection surfaces;
    FixtureGroupCollection groups;
    BusGraph *bus;
    SceneManager *sceneManager;
    QList<ProjectResource*> selectedResources;
    // The SceneObject-only slice of selectedResources, kept in step by
    // setSelectedResources().
    QList<SceneObject*> selectedSceneObjects;
    QWidget *propertiesWidget = nullptr;
};

Project::Impl::Impl()
{
    sceneManager = new SceneManager;

    DMXGenerateMatrixNode *generateNode = new DMXGenerateMatrixNode;
    generateNode->createParameters();

    // Hosts Fixture State nodes that set fixtures' initial values before the
    // Surface node's live controls run.
    DMXSubGraphNode *initialValuesNode = new DMXSubGraphNode;
    initialValuesNode->setName("Initial Values");
    initialValuesNode->createParameters();
    initialValuesNode->setPosition(QPointF(300,0));

    SurfaceNode *sequenceNode = new SurfaceNode;
    sequenceNode->createParameters();
    sequenceNode->setPosition(QPointF(600,0));

    // The project owns surfaces; the node only references one by id.
    Surface *defaultSurface = new Surface("Surface");
    surfaces.addSurface(defaultSurface);
    sequenceNode->setSurfaceId(defaultSurface->uniqueId());

    // Last in the chain before the output node — overrides one fixture's
    // dimmer/shutter/color when the DMX Patch panel's Identify toggle is on.
    IdentifyFixtureNode *identifyNode = new IdentifyFixtureNode;
    identifyNode->setName("Identify");
    identifyNode->createParameters();
    identifyNode->setPosition(QPointF(750,0));

    DMXWriterNode *writerNode = new DMXWriterNode;
    writerNode->setName("output");
    writerNode->createParameters();
    writerNode->setPosition(QPointF(900,0));

    bus = new BusGraph;

    bus->addNode(generateNode);
    bus->addNode(initialValuesNode);
    bus->addNode(sequenceNode);
    bus->addNode(identifyNode);
    bus->addNode(writerNode);

    bus->connectParameters(generateNode->findParameter(DMXGenerateMatrixNode::OutputDMX), initialValuesNode->findParameter(DMXSubGraphNode::InputDMX));
    bus->connectParameters(initialValuesNode->findParameter(DMXSubGraphNode::OutputDMX), sequenceNode->findParameter(SurfaceNode::InputDMX));
    bus->connectParameters(sequenceNode->findParameter(SurfaceNode::OutputDMX), identifyNode->findParameter(IdentifyFixtureNode::InputDMX));
    bus->connectParameters(identifyNode->findParameter(IdentifyFixtureNode::OutputDMX), writerNode->findParameter(DMXWriterNode::InputDMX));

    bus->drainCommandQueue();

    watchBus();
}

Project::Impl::~Impl()
{
    delete bus;
}

void Project::Impl::adoptSurfaceNode(keira::Node *t_node)
{
    auto *surfaceNode = dynamic_cast<SurfaceNode*>(t_node);
    if(!surfaceNode || !surfaceNode->surfaceId().isEmpty())
        return;

    Surface *surface = new Surface("Surface");
    surfaces.addSurface(surface);
    surfaceNode->setSurfaceId(surface->uniqueId());
}

void Project::Impl::watchBus()
{
    // nodeWasAdded can fire on the eval thread (addNodeInternal runs from
    // drainCommandQueue). Passing `bus` as the receiver context makes this a
    // queued connection in that case, so the surface is created on the GUI
    // thread where the collection's signals are consumed. The node resolves its
    // id lazily, so it just sees no surface until then.
    QObject::connect(bus, &keira::Graph::nodeWasAdded, bus, [this](keira::Node *node){
        adoptSurfaceNode(node);
    });
}

Project::Project(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

Project::~Project()
{
    delete m_impl->sceneManager;
    delete m_impl;
}

BusGraph *Project::bus() const
{
    return m_impl->bus;
}

SceneObject *Project::sceneRoot() const
{
    return m_impl->sceneManager->rootObject();
}

ProjectResource *Project::selectedResource() const
{
    return m_impl->selectedResources.isEmpty() ? nullptr : m_impl->selectedResources.last();
}

void Project::setSelectedResource(ProjectResource *t_resource)
{
    setSelectedResources(t_resource ? QList<ProjectResource*>{t_resource} : QList<ProjectResource*>{});
}

QList<ProjectResource*> Project::selectedResources() const
{
    return m_impl->selectedResources;
}

void Project::setSelectedResources(const QList<ProjectResource*> &t_resources)
{
    if(t_resources == m_impl->selectedResources)
        return;

    m_impl->selectedResources = t_resources;

    // Scene-object consumers see only their slice of the selection. Selecting a
    // routine therefore clears the scene selection rather than leaving the
    // visualizer highlighting something no longer selected.
    QList<SceneObject*> sceneObjects;
    for(auto *resource : t_resources)
        if(auto *sceneObject = dynamic_cast<SceneObject*>(resource))
            sceneObjects.append(sceneObject);

    const bool sceneSelectionChanged = sceneObjects != m_impl->selectedSceneObjects;
    m_impl->selectedSceneObjects = sceneObjects;

    emit selectedResourcesChanged(t_resources);
    emit selectedResourceChanged(t_resources.isEmpty() ? nullptr : t_resources.last());

    if(sceneSelectionChanged)
    {
        emit selectedSceneObjectsChanged(sceneObjects);
        emit selectedSceneObjectChanged(sceneObjects.isEmpty() ? nullptr : sceneObjects.last());
    }
}

SceneObject *Project::selectedSceneObject() const
{
    return m_impl->selectedSceneObjects.isEmpty() ? nullptr : m_impl->selectedSceneObjects.last();
}

void Project::setSelectedSceneObject(SceneObject *obj)
{
    setSelectedSceneObjects(obj ? QList<SceneObject*>{obj} : QList<SceneObject*>{});
}

QList<SceneObject*> Project::selectedSceneObjects() const
{
    return m_impl->selectedSceneObjects;
}

void Project::setSelectedSceneObjects(const QList<SceneObject*> &obj)
{
    QList<ProjectResource*> resources;
    resources.reserve(obj.size());
    for(auto *sceneObject : obj)
        resources.append(sceneObject);

    setSelectedResources(resources);
}

QWidget *Project::propertiesWidget() const
{
    return m_impl->propertiesWidget;
}

void Project::setPropertiesWidget(QWidget *t_widget)
{
    if(m_impl->propertiesWidget == t_widget)
        return;
    QWidget *old = m_impl->propertiesWidget;
    m_impl->propertiesWidget = t_widget;
    // The panel detaches `old` from its layout in response to this signal, so it is
    // safe to delete afterwards. deleteLater avoids freeing it mid-signal.
    emit propertiesWidgetChanged(t_widget);
    if(old)
        old->deleteLater();
}

SceneManager *Project::scene() const
{
    return m_impl->sceneManager;
}

RoutineCollection *Project::routines() const
{
    return &m_impl->routines;
}

FixtureCollection *Project::fixtures() const
{
    return &m_impl->fixtures;
}

QStringList Project::allTags() const
{
    QSet<QString> tags;

    for(auto *object : SceneIterator::ToList(sceneRoot()))
        for(const auto &tag : object->resourceTags())
            tags.insert(tag);

    for(auto *group : m_impl->groups.groups())
        for(const auto &tag : group->resourceTags())
            tags.insert(tag);

    for(auto *routine : m_impl->routines.routines())
        for(const auto &tag : routine->resourceTags())
            tags.insert(tag);

    for(auto *surface : m_impl->surfaces.surfaces())
        for(const auto &tag : surface->resourceTags())
            tags.insert(tag);

    for(auto *layout : m_impl->pixelLayouts.layouts())
        for(const auto &tag : layout->resourceTags())
            tags.insert(tag);

    // Sequences are owned by PhotonCore, not the project - guarded the same
    // way ProjectModel guards it, since a bare Project (as in tests) has no
    // running application.
    if(photonApp && photonApp->sequences())
        for(auto *sequence : photonApp->sequences()->sequences())
            for(const auto &tag : sequence->resourceTags())
                tags.insert(tag);

    QStringList sorted(tags.constBegin(), tags.constEnd());
    sorted.sort();
    return sorted;
}

FixtureGroupCollection *Project::groups() const
{
    return &m_impl->groups;
}

SurfaceCollection *Project::surfaces() const
{
    return &m_impl->surfaces;
}

PixelLayoutCollection *Project::pixelLayouts() const
{
    return &m_impl->pixelLayouts;
}

void Project::save(const QString &path) const
{
    QSettings qsettings;

    qsettings.beginGroup("app");
    QString startPath = qsettings.value("savepath", QDir::homePath()).toString();
    qsettings.endGroup();

    QString savePath = path;
    if(savePath.isEmpty())
    {
        savePath = QFileDialog::getSaveFileName(nullptr,"Save Project", startPath, "Photon Project (*.proj)");
    }

    if(savePath.isEmpty())
    {
        qWarning("There was no path to save to.");
        return;
    }

    QFile saveFile(savePath);

    if (!saveFile.open(QIODevice::WriteOnly)) {
             qWarning("Couldn't open save file.");
             return;
         }

    qsettings.beginGroup("app");
    qsettings.setValue("savepath", QFileInfo(savePath).path());
    qsettings.endGroup();

    QJsonObject jsonObj;
    writeToJson(jsonObj);

    saveFile.write(QJsonDocument(jsonObj).toJson());

    qDebug() << "Saved to: " << saveFile.fileName();

    qsettings.beginGroup("app");
    qsettings.setValue("loadpath", QFileInfo(savePath).path());
    qsettings.setValue("lastproject", savePath);
    qsettings.endGroup();

}

void Project::load(const QString &path)
{
    QString loadPath = path;
    if(loadPath.isNull())
    {
        QSettings qsettings;
        qsettings.beginGroup("app");
        QString startPath = qsettings.value("loadpath", QDir::homePath()).toString();
        qsettings.endGroup();

        loadPath = QFileDialog::getOpenFileName(nullptr, "Photon Project",
                                            startPath,
                                            "*.proj");

        if(loadPath.isNull())
            return;

    }


    QFile loadFile(loadPath);

    if (!loadFile.open(QIODevice::ReadOnly)) {
             qWarning("Couldn't open load file.");
             return;
         }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    readFromJson(loadDoc.object());
    restore(*this);

    qDebug() << "Load from: " << loadFile.fileName();
}

void Project::restore(Project &t_project)
{
    for(auto surface : m_impl->surfaces.surfaces())
    {
        surface->restore(t_project);
    }
}

void Project::readFromJson(const QJsonObject &json)
{
    LoadContext context;
    context.project = this;

    // The constructor seeds a default surface so a brand-new project is
    // usable. Loading into that same instance would leave it orphaned
    // alongside the saved ones, so drop it first.
    m_impl->surfaces.clear();

    if(json.contains("fixtureGroups"))
        m_impl->groups.readFromJson(json.value("fixtureGroups").toObject());

    //m_impl->sceneManager = new SceneManager;
    if(json.contains("sceneManager"))
    {
        QJsonObject sceneObj = json.value("sceneManager").toObject();
        m_impl->sceneManager->readFromJson(sceneObj, context);
    }



    if(json.contains("pixelLayouts"))
    {
        QJsonArray layoutArray = json.value("pixelLayouts").toArray();
        for(const auto &layout : layoutArray)
        {
            const QJsonObject &layoutObj = layout.toObject();

            PixelLayout *c = new PixelLayout;
            c->readFromJson(layoutObj, context);
            m_impl->pixelLayouts.addLayout(c);
        }
    }

    if(json.contains("routines"))
    {
        QJsonArray routineArray = json.value("routines").toArray();
        for(const auto &rout : routineArray)
        {
            const QJsonObject &routineObj = rout.toObject();

            Routine *routine = new Routine;
            routine->readFromJson(routineObj, photonApp->plugins()->nodeLibrary());
            m_impl->routines.addRoutine(routine);
        }
    }

    // Must precede the bus graph: SurfaceNodes resolve their surface by id
    // against this collection.
    if(json.contains("surfaces"))
    {
        QJsonArray surfaceArray = json.value("surfaces").toArray();
        for(const auto &surf : surfaceArray)
        {
            const QJsonObject &surfaceObj = surf.toObject();

            Surface *surface = new Surface;
            surface->readFromJson(surfaceObj, context);
            m_impl->surfaces.addSurface(surface);
        }
    }

    delete m_impl->bus;
    m_impl->bus = new BusGraph;
    m_impl->watchBus();
    if(json.contains("bus"))
    {
        QJsonObject busObj = json.value("bus").toObject();
        m_impl->bus->readFromJson(busObj, photonApp->plugins()->nodeLibrary());
    }

    // Projects saved before the Identify node existed won't have one in their
    // bus graph — splice one in between the Surface node and the output node
    // so upgraded projects still get the DMX Patch panel's identify feature.
    if(!m_impl->bus->findNode("Identify"))
    {
        auto *surfaceNode = m_impl->bus->findNode("Surface");
        auto *outputNode = m_impl->bus->findNode("output");
        if(surfaceNode && outputNode)
        {
            auto *surfaceOut = surfaceNode->findParameter(SurfaceNode::OutputDMX);
            auto *writerIn = outputNode->findParameter(DMXWriterNode::InputDMX);
            if(surfaceOut && writerIn)
            {
                auto *identifyNode = new IdentifyFixtureNode;
                identifyNode->setName("Identify");
                identifyNode->createParameters();
                identifyNode->setPosition(surfaceNode->position() + QPointF(150, 100));

                m_impl->bus->addNode(identifyNode);
                m_impl->bus->disconnectParameters(surfaceOut, writerIn);
                m_impl->bus->connectParameters(surfaceOut, identifyNode->findParameter(IdentifyFixtureNode::InputDMX));
                m_impl->bus->connectParameters(identifyNode->findParameter(IdentifyFixtureNode::OutputDMX), writerIn);
                m_impl->bus->drainCommandQueue();
            }
        }
    }
}

void Project::writeToJson(QJsonObject &json) const
{
    QJsonObject busObj;
    m_impl->bus->writeToJson(busObj);
    json.insert("bus", busObj);

    QJsonArray routineArray;
    for(auto routine : m_impl->routines.routines())
    {
        QJsonObject routineObj;
        routine->writeToJson(routineObj);
        routineArray.append(routineObj);
    }
    json.insert("routines", routineArray);

    QJsonObject groupsObj;
    m_impl->groups.writeToJson(groupsObj);
    json.insert("fixtureGroups", groupsObj);

    QJsonArray layoutArray;
    for(auto layout : m_impl->pixelLayouts.layouts())
    {
        QJsonObject layoutObj;
        layout->writeToJson(layoutObj);
        layoutArray.append(layoutObj);
    }
    json.insert("pixelLayouts", layoutArray);

    QJsonObject sceneObj;
    m_impl->sceneManager->writeToJson(sceneObj);
    json.insert("sceneManager", sceneObj);

    QJsonArray surfacesArray;
    for(auto surface : m_impl->surfaces.surfaces())
    {
        QJsonObject surfaceObj;
        surface->writeToJson(surfaceObj);
        surfacesArray.append(surfaceObj);
    }
    json.insert("surfaces", surfacesArray);
}

} // namespace photon
