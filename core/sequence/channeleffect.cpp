#include "channeleffect_p.h"
#include "channel.h"

namespace photon {



ChannelEffect::ChannelEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
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

void ChannelEffect::restore(Project &)
{

}

void ChannelEffect::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
}

void ChannelEffect::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
}

} // namespace photon
