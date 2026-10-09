#include <algorithm>
#include "layer_p.h"
#include "sequence.h"

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

bool Layer::isMuted() const
{
    return m_impl->muted;
}

void Layer::setMuted(bool t_muted)
{
    if(m_impl->muted == t_muted)
        return;
    m_impl->muted = t_muted;
    emit metadataChanged();
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
    m_impl->muted = t_json.value("muted").toBool();
}

void Layer::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("guid", m_impl->guid.toString());
    t_json.insert("type", QString(m_impl->type));
    t_json.insert("muted", m_impl->muted);

}


QColor Layer::defaultClipColor() const
{
    // Distinct, mid-bright hues that read on the dark timeline; the first
    // layer keeps the old red default.
    static const QVector<QColor> palette = {
        QColor(214, 69, 65),    // red
        QColor(230, 145, 56),   // orange
        QColor(214, 196, 60),   // yellow
        QColor(96, 184, 87),    // green
        QColor(58, 175, 169),   // teal
        QColor(72, 133, 214),   // blue
        QColor(142, 98, 204),   // purple
        QColor(212, 88, 158),   // pink
    };
    const int index = sequence() ? int(sequence()->layers().indexOf(const_cast<Layer*>(this))) : 0;
    return palette[std::max(index, 0) % palette.size()];
}

} // namespace photon
