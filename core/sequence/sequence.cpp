#include "sequence_p.h"
#include "layer_p.h"
#include "cliplayer.h"
#include "masterlayer.h"
#include "canvaslayergroup.h"
#include "project/project.h"
#include "photoncore.h"

namespace photon {

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
}

Sequence::~Sequence()
{
    delete m_impl;
}

void Sequence::init()
{
    addLayer(new ClipLayer("Layer 1"));
}

void Sequence::addBeatLayer(BeatLayer *t_layer)
{
    qDebug() << "Add" << t_layer->beats().length();
    m_impl->beatLayers.append(t_layer);
    emit beatLayerAdded(t_layer);
}

void Sequence::removeBeatLayer(BeatLayer *t_layer)
{
    m_impl->beatLayers.removeOne(t_layer);
    emit beatLayerRemoved(t_layer);
}

bool Sequence::findClosestBeatToTime(float t_time, float *t_result) const
{
    *t_result = 0.0f;
    if(m_impl->beatLayers.isEmpty())
        return false;

    auto layer = m_impl->beatLayers.front();
    auto beats = layer->beats();

    auto it = std::lower_bound(beats.begin(), beats.end(), t_time);

    if (it == beats.begin()) {
            *t_result = beats.front();
    }

    double a = *(it - 1);
    double b = *(it);

    if (fabs(t_time - a) < fabs(t_time - b)) {
        *t_result = *(it - 1);
    }
    else
        *t_result = *it;


    return true;
}

bool Sequence::snapToBeat(float time, float *outTime, float tolerance) const
{
    *outTime = time;
    bool hasSnap = false;
    float winner = 100000000.f;
    for(auto beatLayer : m_impl->beatLayers)
    {
        if(beatLayer->isSnappable())
        {
            float snapTime = 0;
            if(beatLayer->snapToBeat(time, &snapTime, tolerance))
            {
                if(abs(snapTime - time) < winner)
                {
                    winner = snapTime;
                    hasSnap = true;
                }
            }
        }
    }

    if(hasSnap)
    {
        *outTime = winner;
    }
    return hasSnap;
}

const QVector<BeatLayer*> &Sequence::beatLayers() const
{
    return m_impl->beatLayers;
}

BeatLayer *Sequence::editableBeatLayer() const
{
    for(auto beatLayer : m_impl->beatLayers)
    {
        if(beatLayer->isEditable())
            return beatLayer;
    }
    return nullptr;
}

void Sequence::setEditableBeatLayer(BeatLayer *t_layer)
{
    for(auto beatLayer : m_impl->beatLayers)
    {
        if(beatLayer->isEditable())
            beatLayer->setIsEditable(false);
    }
    t_layer->setIsEditable(true);
    emit editableBeatLayerChanged(t_layer);
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

const QVector<Layer*> &Sequence::layers() const
{
    return m_impl->layers;
}

QString Sequence::name() const
{
    return m_impl->name;
}

void Sequence::setName(const QString &t_value)
{
    m_impl->name = t_value;
}

QString Sequence::filePath() const
{
    return m_impl->filePath;
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
    for(auto layer : m_impl->layers)
        layer->processChannels(t_context);
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
        if(layerObj.value("type").toString() == "MasterLayer")
        {
            auto layer = new MasterLayer(this);
            m_impl->addLayer(layer);
            layer->readFromJson(layerObj, t_context);
        }
        if(layerObj.value("type").toString() == "CanvasGroup")
        {
            auto layer = new CanvasLayerGroup(this);
            m_impl->addLayer(layer);
            layer->readFromJson(layerObj, t_context);
        }
    }
    auto beatArray = t_json.value("beatLayers").toArray();
    for(auto layerJson : beatArray)
    {
        auto layerObj = layerJson.toObject();
        auto beatLayer = new BeatLayer;
        beatLayer->readFromJson(layerObj, t_context);
        m_impl->beatLayers.append(beatLayer);
    }

    if(!m_impl->beatLayers.isEmpty())
        m_impl->beatLayers.front()->setIsEditable(true);
}

void Sequence::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("filePath", m_impl->filePath);

    QJsonArray array;
    for(auto layer : m_impl->layers)
    {
        QJsonObject layerJson;
        layer->writeToJson(layerJson);
        array.append(layerJson);
    }
    t_json.insert("layers", array);

    QJsonArray beatArray;
    for(auto layer : m_impl->beatLayers)
    {
        QJsonObject layerJson;
        layer->writeToJson(layerJson);
        beatArray.append(layerJson);
    }
    t_json.insert("beatLayers", beatArray);
}


} // namespace photon
