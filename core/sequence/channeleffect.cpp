#include "channeleffect_p.h"
#include "channel.h"

namespace photon {


void ChannelEffect::Impl::addToChannel(Channel *t_channel)
{
    channel = t_channel;
    facade->addedToChannel();
}

ChannelEffect::ChannelEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->facade = this;
}

ChannelEffect::~ChannelEffect()
{
    delete m_impl;
}

Channel *ChannelEffect::channel() const
{
    return m_impl->channel;
}

int ChannelEffect::index() const
{
    return 0;
}

void ChannelEffect::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray ChannelEffect::id() const
{
    return m_impl->id;
}

QByteArray ChannelEffect::uniqueId() const
{
    return m_impl->uniqueId;
}

void ChannelEffect::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString ChannelEffect::name() const
{
    return m_impl->name;
}

void ChannelEffect::updated()
{
    if(m_impl->channel)
        m_impl->channel->effectUpdated(this);
}

ChannelEffect *ChannelEffect::previousEffect() const
{
    return m_impl->previousEffect;
}

double ChannelEffect::process(double, double) const
{
    return 0.0;
}

QColor ChannelEffect::processColor(QColor, double) const
{
    return QColor();
}

void ChannelEffect::restore(Project &)
{

}

void ChannelEffect::addedToChannel()
{

}

ChannelEffectViewState ChannelEffect::viewState() const
{
    return m_impl->viewState;
}

void ChannelEffect::setViewState(const ChannelEffectViewState &t_state)
{
    m_impl->viewState = t_state;
}

void ChannelEffect::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString(QUuid::createUuid().toString()).toLatin1();
}

void ChannelEffect::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
}

} // namespace photon
