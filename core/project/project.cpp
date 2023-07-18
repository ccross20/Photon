#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include "project.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "pixel/canvascollection.h"
#include "pixel/canvas.h"
#include "sequence/sequencecollection.h"
#include "routine/routinecollection.h"
#include "routine/routine.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "sequence/sequence.h"
#include "graph/bus/busgraph.h"
#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/sequencenode.h"
#include "graph/bus/dmxwriternode.h"
#include "scene/sceneobject.h"
#include "scene/scenemanager.h"

namespace photon {

class Project::Impl
{
public:
    Impl();
    ~Impl();
    CanvasCollection canvases;
    PixelLayoutCollection pixelLayouts;
    FixtureCollection fixtures;
    SequenceCollection sequences;
    RoutineCollection routines;
    BusGraph *bus;
    SceneManager *sceneManager;
};

Project::Impl::Impl()
{
    sceneManager = new SceneManager;

    DMXGenerateMatrixNode *generateNode = new DMXGenerateMatrixNode;
    generateNode->createParameters();

    SequenceNode *sequenceNode = new SequenceNode;
    sequenceNode->createParameters();
    sequenceNode->setPosition(QPointF(400,0));

    DMXWriterNode *writerNode = new DMXWriterNode;
    writerNode->setName("output");
    writerNode->createParameters();
    writerNode->setPosition(QPointF(800,0));

    bus = new BusGraph;

    bus->addNode(generateNode);
    bus->addNode(sequenceNode);
    bus->addNode(writerNode);

    bus->connectParameters(generateNode->findParameter(DMXGenerateMatrixNode::OutputDMX), sequenceNode->findParameter(SequenceNode::InputDMX));
    bus->connectParameters(sequenceNode->findParameter(SequenceNode::OutputDMX), writerNode->findParameter(DMXWriterNode::InputDMX));


}

Project::Impl::~Impl()
{
    delete bus;
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

SceneManager *Project::scene() const
{
    return m_impl->sceneManager;
}

void Project::addSequence(Sequence *t_sequence)
{
    m_impl->sequences.addSequence(t_sequence);
    t_sequence->setParent(this);
}

RoutineCollection *Project::routines() const
{
    return &m_impl->routines;
}

SequenceCollection *Project::sequences() const
{
    return &m_impl->sequences;
}

FixtureCollection *Project::fixtures() const
{
    return &m_impl->fixtures;
}

CanvasCollection *Project::canvases() const
{
    return &m_impl->canvases;
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
        qsettings.beginGroup("app");
        qsettings.setValue("loadpath", QFileInfo(loadPath).path());
        qsettings.endGroup();

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
    for(auto sequence : m_impl->sequences.sequences())
        sequence->restore(t_project);
}

void Project::readFromJson(const QJsonObject &json)
{
    LoadContext context;
    context.project = this;

    //m_impl->sceneManager = new SceneManager;
    if(json.contains("sceneManager"))
    {
        QJsonObject sceneObj = json.value("sceneManager").toObject();
        m_impl->sceneManager->readFromJson(sceneObj);
    }

    m_impl->bus = new BusGraph;
    if(json.contains("bus"))
    {
        QJsonObject busObj = json.value("bus").toObject();
        m_impl->bus->readFromJson(busObj, photonApp->plugins()->nodeLibrary());
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

    if(json.contains("sequences"))
    {
        QJsonArray sequenceArray = json.value("sequences").toArray();
        for(const auto &seq : sequenceArray)
        {
            const QJsonObject &sequenceObj = seq.toObject();

            Sequence *sequence = new Sequence;
            sequence->readFromJson(sequenceObj, context);
            m_impl->sequences.addSequence(sequence);
        }
    }

    if(json.contains("canvases"))
    {
        QJsonArray canvasArray = json.value("canvases").toArray();
        for(const auto &canvas : canvasArray)
        {
            const QJsonObject &canvasObj = canvas.toObject();

            Canvas *c = new Canvas;
            c->readFromJson(canvasObj, context);
            m_impl->canvases.addCanvas(c);
        }
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





}

void Project::writeToJson(QJsonObject &json) const
{
    QJsonObject busObj;
    m_impl->bus->writeToJson(busObj);
    json.insert("bus", busObj);


    QJsonArray sequenceArray;
    for(auto sequence : m_impl->sequences.sequences())
    {
        QJsonObject sequenceObj;
        sequence->writeToJson(sequenceObj);
        sequenceArray.append(sequenceObj);
    }
    json.insert("sequences", sequenceArray);

    QJsonArray routineArray;
    for(auto routine : m_impl->routines.routines())
    {
        QJsonObject routineObj;
        routine->writeToJson(routineObj);
        routineArray.append(routineObj);
    }
    json.insert("routines", routineArray);

    QJsonArray canvasArray;
    for(auto canvas : m_impl->canvases.canvases())
    {
        QJsonObject canvasObj;
        canvas->writeToJson(canvasObj);
        canvasArray.append(canvasObj);
    }
    json.insert("canvases", canvasArray);

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

}

} // namespace photon
