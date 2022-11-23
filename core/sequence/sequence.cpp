#include "sequence_p.h"
#include "layer_p.h"
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
    addLayer(new Layer("Layer 1"));
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

void Sequence::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();

    auto array = t_json.value("layers").toArray();
    for(auto layerJson : array)
    {
        auto layer = new Layer;
        m_impl->addLayer(layer);
        layer->readFromJson(layerJson.toObject(), t_context);
    }
}

void Sequence::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);

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
