#include "layer_p.h"
#include "sequence.h"
#include "layergroup.h"

namespace photon {

Layer::Impl::Impl(Layer *t_facade):facade(t_facade)
{

}

void Layer::Impl::setSequence(Sequence *t_sequence)
{
    sequence = t_sequence;
    facade->sequenceChanged(sequence);
}


Layer::Layer(const QString &t_name, const QByteArray &layerType, QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{
    setName(t_name);
    m_impl->guid = QUuid::createUuid();
    m_impl->type = layerType;
}

Layer::~Layer()
{
    delete m_impl;
}

LayerGroup *Layer::parentGroup() const
{
    return dynamic_cast<LayerGroup*>(parent());
}

QWidget *Layer::createEditor()
{
    return nullptr;
}

QByteArray Layer::uniqueId() const
{
    return m_impl->guid.toByteArray();
}

QUuid Layer::guid() const
{
    return m_impl->guid;
}

Layer *Layer::findLayerByGuid(const QUuid &guid)
{
    if(m_impl->guid == guid)
        return this;
    return nullptr;
}

QString Layer::name() const
{
    return m_impl->name;
}

void Layer::setName(const QString &name)
{
    m_impl->name = name;
    setObjectName(name);
    emit metadataChanged();
}

QByteArray Layer::layerType() const
{
    return m_impl->type;
}

Sequence *Layer::sequence() const
{
    return m_impl->sequence;
}

int Layer::height() const
{
    return 25;
}

void Layer::sequenceChanged(Sequence *)
{

}

void Layer::processChannels(ProcessContext &t_context)
{

}

void Layer::restore(Project &t_project)
{

}

void Layer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->guid = QUuid::fromString(t_json.value("guid").toString());
}

void Layer::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("guid", m_impl->guid.toString());
    t_json.insert("type", QString(m_impl->type));

}


} // namespace photon
