#include "layergroup_p.h"
#include "layer_p.h"
#include "cliplayer.h"
#include "masterlayer.h"

namespace photon {

LayerGroup::LayerGroup(const QByteArray &t_layerType, QObject *t_parent) :
    Layer("", t_layerType, t_parent),m_impl(new Impl)
{

}

LayerGroup::LayerGroup(const QString &t_name, const QByteArray &t_layerType, QObject *t_parent) :
    Layer(t_name, t_layerType, t_parent),m_impl(new Impl)
{
    addLayer(new ClipLayer("Sub Layer"));
}

LayerGroup::~LayerGroup()
{
    delete m_impl;
}

void LayerGroup::addLayer(Layer *t_layer)
{
    m_impl->layers.append(t_layer);
    t_layer->setParent(this);
    t_layer->m_impl->setSequence(sequence());

    emit layerAdded(t_layer);
}

void LayerGroup::removeLayer(Layer *t_layer)
{
    m_impl->layers.removeOne(t_layer);
    t_layer->setParent(nullptr);
    t_layer->m_impl->setSequence(nullptr);
    emit layerRemoved(t_layer);
}

Layer *LayerGroup::findLayerByGuid(const QUuid &t_guid)
{
    for(auto layer : m_impl->layers)
    {
        auto results = layer->findLayerByGuid(t_guid);
        if(results)
            return results;
    }
    return Layer::findLayerByGuid(t_guid);
}

const QVector<Layer*> &LayerGroup::layers() const
{
    return m_impl->layers;
}

int LayerGroup::height() const
{
    int total = 20;
    for(auto layer : m_impl->layers)
    {
        total += layer->height();
    }
    return total;
}

void LayerGroup::processChannels(ProcessContext &t_context)
{
    for(auto layer : m_impl->layers)
        layer->processChannels(t_context);
}

void LayerGroup::restore(Project &t_project)
{
    for(auto layer : m_impl->layers)
        layer->restore(t_project);
}

void LayerGroup::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Layer::readFromJson(t_json, t_context);
    auto array = t_json.value("layers").toArray();
    for(auto layerJson : array)
    {
        auto layerObj = layerJson.toObject();
        if(layerObj.value("type").toString() == "ClipLayer")
        {
            auto layer = new ClipLayer("", this);
            addLayer(layer);
            layer->readFromJson(layerObj, t_context);
        }
        if(layerObj.value("type").toString() == "MasterLayer")
        {
            auto layer = new MasterLayer(this);
            addLayer(layer);
            layer->readFromJson(layerObj, t_context);
        }
    }
}

void LayerGroup::writeToJson(QJsonObject &t_json) const
{
    Layer::writeToJson(t_json);

    QJsonArray array;
    for(auto layer : m_impl->layers)
    {
        QJsonObject layerJson;
        layer->writeToJson(layerJson);
        array.append(layerJson);
    }
    t_json.insert("layers", array);

}


} // namespace photon
