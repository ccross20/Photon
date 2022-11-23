#include <QJsonArray>
#include "channel_p.h"
#include "channeleffect_p.h"
#include "clip.h"
#include "plugin/pluginfactory.h"
#include "photoncore.h"

namespace photon {



Channel::Channel(Clip *t_clip, const ChannelInfo &t_info):m_impl(new Impl)
{
    m_impl->info = t_info;
    m_impl->clip = t_clip;
}

Channel::~Channel()
{
    delete m_impl;
}

ChannelInfo Channel::info() const
{
    return m_impl->info;
}

Clip *Channel::clip() const
{
    return m_impl->clip;
}

void Channel::addEffect(ChannelEffect *t_effect)
{
    t_effect->m_impl->channel = this;
    if(!m_impl->effects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->effects.back();
    m_impl->effects.append(t_effect);
    emit effectAdded(t_effect);
}

void Channel::removeEffect(ChannelEffect *t_effect)
{
    t_effect->m_impl->channel = nullptr;
    m_impl->effects.removeOne(t_effect);
    emit effectRemoved(t_effect);
}

void Channel::moveEffect(ChannelEffect *t_effect, int newPosition)
{
    m_impl->effects.move(t_effect->index(), newPosition);
    emit effectMoved(t_effect);
}

double Channel::processDouble(double time)
{
    double val = m_impl->info.defaultValue.toDouble();

    /*
    for(auto effect : m_impl->effects)
    {
        val = effect->process(val, time);
    }
    */
    if(!m_impl->effects.isEmpty())
        val = m_impl->effects.back()->process(val, time);

    return val;
}

void Channel::effectUpdated(ChannelEffect *t_effect)
{
    m_impl->clip->channelUpdatedSlot(this);
    emit effectModified(t_effect);
}

int Channel::effectCount() const
{
    return m_impl->effects.length();
}

ChannelEffect *Channel::effectAtIndex(int index) const
{
    return m_impl->effects.at(index);
}

void Channel::readFromJson(const QJsonObject &t_json, const LoadContext &)
{
    m_impl->info.name = t_json.value("name").toString();
    m_impl->info.description = t_json.value("description").toString();
    m_impl->info.type = static_cast<ChannelInfo::ChannelType>(t_json.value("type").toInt());
    m_impl->info.defaultValue = t_json.value("defaultValue");

    if(t_json.contains("effects"))
    {
        auto effectArray = t_json.value("effects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createChannelEffect(id.toLatin1());

            if(effect){
                effect->readFromJson(effectObj);
                if(!m_impl->effects.isEmpty())
                    effect->m_impl->previousEffect = m_impl->effects.back();
                m_impl->effects.append(effect);

                effect->m_impl->channel = this;
            }
        }
    }
}

void Channel::writeToJson(QJsonObject &t_json) const
{
    QJsonArray effectArray;
    for(auto effect : m_impl->effects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        effectArray.append(effectObj);
    }
    t_json.insert("effects", effectArray);

    t_json.insert("name",m_impl->info.name);
    t_json.insert("description",m_impl->info.description);
    t_json.insert("type",m_impl->info.type);
    t_json.insert("defaultValue",m_impl->info.defaultValue.toJsonValue());

    switch(m_impl->info.type)
    {
    case photon::ChannelInfo::ChannelTypeNumber:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toDouble());
        break;
    case photon::ChannelInfo::ChannelTypeColor:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toJsonValue());
        break;

    }


}


} // namespace photon
