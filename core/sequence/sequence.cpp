#include <cmath>
#include <QJsonDocument>
#include <QFile>
#include "sequence_p.h"
#include "layer_p.h"
#include "cliplayer.h"
#include "clip.h"
#include "project/project.h"
#include "photoncore.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "state/stateevaluationcontext.h"
#include "state/state.h"
#include "library/songlibrary.h"
#include "gui/resourceeditorwidget.h"
#include "sequence/sequencecollection.h"

namespace photon {

namespace {
    // The song-analysis sidecar sits next to the .seq, sharing its base name.
    QString songDataSidecarPath(const QString &t_sequencePath)
    {
        if(t_sequencePath.isEmpty())
            return QString();
        const QFileInfo info(t_sequencePath);
        return info.dir().filePath(info.completeBaseName() + ".song");
    }
}

Sequence::Impl::Impl(Sequence *t_facade):facade(t_facade)
{

}

void Sequence::Impl::addLayer(Layer *t_layer)
{
    layers.append(t_layer);
    t_layer->setParent(facade);
    t_layer->m_impl->setSequence(facade);
}

void Sequence::Impl::removeLayer(Layer *t_layer)
{
    layers.removeAll(t_layer);
    t_layer->setParent(nullptr);
    t_layer->m_impl->setSequence(nullptr);
}


Sequence::Sequence(const QString &t_name, QObject *parent)
    : QObject{parent},m_impl(new Impl(this))
{
    m_impl->name = t_name;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

Sequence::~Sequence()
{
    delete m_impl;
}

void Sequence::init()
{
    addLayer(new ClipLayer("Layer 1"));
}

void Sequence::save(const QString &t_path) const
{
    // No file-dialog fallback: every real caller is Song-Library-driven and
    // already knows its own .seq path (auto-derived, never hand-picked for a
    // new sequence) - a project-embedded sequence never calls save() at all
    // (see Project::writeToJson).
    if(t_path.isEmpty())
    {
        qWarning("Sequence::save() called with no path.");
        return;
    }

    // The actual path being written to always wins - filePath() must reflect
    // reality (e.g. PhotonCore::loadSequence()'s duplicate-tab check compares
    // against it) rather than whatever was last embedded in the JSON below.
    m_impl->filePath = t_path;

    QFile saveFile(t_path);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject jsonObj;
    writeToJson(jsonObj);

    // Which library song this sequence belongs to, so a library copied to
    // another computer can relink it (SongLibrary::importFromFolder()).
    {
        SongLibrary *library = photonApp->songLibrary();
        SongLibraryEntry *song = (library && library->isOpen())
            ? library->findSongBySequencePath(t_path) : nullptr;
        if(song)
            jsonObj.insert("songTrackKey", QString::fromLatin1(song->trackKey));
    }

    saveFile.write(QJsonDocument(jsonObj).toJson());

    qDebug() << "Saved to: " << saveFile.fileName();

    // If this sequence is linked to a Song Library song, its SongData lives there
    // (keyed by trackKey, shared with every other sequence for that song) - saving
    // a second copy into a local sidecar would just be a redundant, driftable
    // duplicate. Only sequences with no library link still use the sidecar.
    SongLibrary *library = photonApp->songLibrary();
    SongLibraryEntry *linkedSong = (library && library->isOpen())
        ? library->findSongBySequencePath(t_path) : nullptr;

    if(linkedSong)
    {
        library->saveSongData(*linkedSong, m_impl->songData);
    }
    else
    {
        // Write the song-analysis sidecar alongside (binary). Skip when there's
        // nothing to store so we don't leave empty .song files next to plain
        // sequences.
        const QString sidecar = songDataSidecarPath(t_path);
        if(!sidecar.isEmpty())
        {
            if(!m_impl->songData.isEmpty())
                m_impl->songData.save(sidecar);
            else if(QFile::exists(sidecar))
                QFile::remove(sidecar);
        }
    }
}

void Sequence::load(const QString &t_path)
{
    // No file-dialog fallback: every real caller already knows its own .seq
    // path - a project-embedded sequence never calls load() at all (see
    // Project::readFromJson).
    if(t_path.isEmpty())
    {
        qWarning("Sequence::load() called with no path.");
        return;
    }

    QFile loadFile(t_path);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open load file.");
        return;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    LoadContext context;
    context.project = photonApp->project();

    readFromJson(loadDoc.object(), context);

    // The path actually opened always wins over whatever "filePath" was
    // embedded in the JSON (which readFromJson just set m_impl->filePath to) -
    // that embedded value can be stale/empty (e.g. the file was renamed since
    // its last save) and PhotonCore::loadSequence()'s duplicate-tab check
    // relies on filePath() matching the real path.
    m_impl->filePath = t_path;

    restore(*photonApp->project());

    // See the matching comment in save(): a library-linked sequence's SongData
    // comes from the library, not a local sidecar (ignore any pre-existing
    // sidecar file in that case - it's a harmless orphan, not the source of truth).
    m_impl->songData = SongData();
    SongLibrary *library = photonApp->songLibrary();
    SongLibraryEntry *linkedSong = (library && library->isOpen())
        ? library->findSongBySequencePath(t_path) : nullptr;

    if(linkedSong)
    {
        m_impl->songData = library->songDataFor(*linkedSong);
    }
    else
    {
        const QString sidecar = songDataSidecarPath(t_path);
        if(!sidecar.isEmpty() && QFile::exists(sidecar))
            m_impl->songData.load(sidecar);
    }

    qDebug() << "Load from: " << loadFile.fileName();
}

void Sequence::addCueLayer(CueLayer *t_layer)
{
    qDebug() << "Add" << t_layer->markers().length();
    m_impl->cueLayers.append(t_layer);
    emit cueLayerAdded(t_layer);
}

void Sequence::removeCueLayer(CueLayer *t_layer)
{
    m_impl->cueLayers.removeOne(t_layer);
    emit cueLayerRemoved(t_layer);
}

bool Sequence::snapTime(float t_time, float *t_outTime, float t_tolerance, const QVector<Clip*> &t_excludeClips) const
{
    *t_outTime = t_time;
    float bestDistance = t_tolerance;
    bool hasSnap = false;
    auto consider = [&](float t_candidate) {
        const float distance = std::abs(t_candidate - t_time);
        if(distance <= bestDistance)
        {
            bestDistance = distance;
            *t_outTime = t_candidate;
            hasSnap = true;
        }
    };

    for(auto cueLayer : m_impl->cueLayers)
    {
        float markerTime = 0;
        if(cueLayer->isSnappable() && cueLayer->snapToMarker(t_time, &markerTime, t_tolerance))
            consider(markerTime);
    }

    for(auto layer : m_impl->layers)
    {
        auto clipLayer = dynamic_cast<ClipLayer*>(layer);
        if(!clipLayer)
            continue;

        for(auto clip : clipLayer->clips())
        {
            if(t_excludeClips.contains(clip))
                continue;
            consider(static_cast<float>(clip->startTime()));
            consider(static_cast<float>(clip->endTime()));
        }
    }

    return hasSnap;
}

const QVector<CueLayer*> &Sequence::cueLayers() const
{
    return m_impl->cueLayers;
}

CueLayer *Sequence::editableCueLayer() const
{
    for(auto cueLayer : m_impl->cueLayers)
    {
        if(cueLayer->isEditable())
            return cueLayer;
    }
    return nullptr;
}

void Sequence::setEditableCueLayer(CueLayer *t_layer)
{
    for(auto cueLayer : m_impl->cueLayers)
    {
        if(cueLayer->isEditable())
            cueLayer->setIsEditable(false);
    }
    t_layer->setIsEditable(true);
    emit editableCueLayerChanged(t_layer);
}

Layer *Sequence::findLayerByGuid(const QUuid &t_guid)
{
    for(auto layer : m_impl->layers)
    {
        auto results = layer->findLayerByGuid(t_guid);
        if(results)
            return results;
    }
    return nullptr;
}

Project *Sequence::project() const
{
    return photonApp->project();
}

SongData *Sequence::songData() const
{
    return &m_impl->songData;
}

double Sequence::previewTime() const
{
    return m_impl->previewTime.load(std::memory_order_relaxed);
}

void Sequence::setPreviewTime(double t_value)
{
    m_impl->previewTime.store(t_value, std::memory_order_relaxed);
}

const QVector<Layer*> &Sequence::layers() const
{
    return m_impl->layers;
}

QWidget *Sequence::createResourceEditor()
{
    auto *editor = new ResourceEditorWidget(this);
    editor->setOpenAction("Open Sequence", [this](){
        photonApp->sequences()->editSequence(this);
    });
    return editor;
}

QString Sequence::name() const
{
    return m_impl->name;
}

void Sequence::setName(const QString &t_value)
{
    if(m_impl->name == t_value)
        return;
    m_impl->name = t_value;
    notifyResourceChanged();
}

QByteArray Sequence::uniqueId() const
{
    return m_impl->uniqueId;
}

QString Sequence::filePath() const
{
    return m_impl->filePath;
}

bool Sequence::isLibrarySequence() const
{
    return m_impl->isLibrarySequence;
}

void Sequence::setIsLibrarySequence(bool t_value)
{
    m_impl->isLibrarySequence = t_value;
}

void Sequence::setAudioPath(const QString &t_path)
{
    if(m_impl->filePath == t_path)
        return;
    m_impl->filePath = t_path;

    qDebug() << t_path;

    emit fileChanged(m_impl->filePath);
}

void Sequence::addLayer(Layer *t_layer)
{
    m_impl->addLayer(t_layer);

    emit layerAdded(t_layer);
}

void Sequence::removeLayer(Layer *t_layer)
{
    m_impl->removeLayer(t_layer);

    emit layerRemoved(t_layer);
}


void Sequence::processChannels(ProcessContext &t_context, double lastTime)
{
    StateEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = t_context.globalTime;


    for(auto layer : m_impl->layers)
    {
        if(!layer->isMuted())
            layer->processChannels(t_context);
    }

    //qDebug() << t_context.dmxMatrix.value(0,3);
}

void Sequence::restore(Project &t_project)
{
    for(auto layer : m_impl->layers)
        layer->restore(t_project);
}

void Sequence::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->filePath = t_json.value("filePath").toString();
    // Sequences predate having an id, so keep the generated one for files
    // written before this existed rather than blanking it.
    if(t_json.contains("uniqueId"))
        m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    readResourceJson(t_json);

    auto array = t_json.value("layers").toArray();
    for(auto layerJson : array)
    {
        auto layerObj = layerJson.toObject();
        if(layerObj.value("type").toString() == "ClipLayer")
        {
            auto layer = new ClipLayer("", this);
            m_impl->addLayer(layer);
            layer->readFromJson(layerObj, t_context);
        }
    }
    auto cueArray = t_json.value("cueLayers").toArray();
    for(auto layerJson : cueArray)
    {
        auto layerObj = layerJson.toObject();
        auto cueLayer = new CueLayer;
        cueLayer->readFromJson(layerObj, t_context);
        m_impl->cueLayers.append(cueLayer);
    }

    if(!m_impl->cueLayers.isEmpty())
        m_impl->cueLayers.front()->setIsEditable(true);
}

void Sequence::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("filePath", m_impl->filePath);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    writeResourceJson(t_json);

    QJsonArray array;
    for(auto layer : m_impl->layers)
    {
        QJsonObject layerJson;
        layer->writeToJson(layerJson);
        array.append(layerJson);
    }
    t_json.insert("layers", array);

    QJsonArray cueArray;
    for(auto layer : m_impl->cueLayers)
    {
        QJsonObject layerJson;
        layer->writeToJson(layerJson);
        cueArray.append(layerJson);
    }
    t_json.insert("cueLayers", cueArray);
}


} // namespace photon
