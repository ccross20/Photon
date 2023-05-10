#include <QJsonArray>
#include <QUuid>
#include "channel_p.h"
#include "channeleffect_p.h"
#include "sequence.h"
#include "plugin/pluginfactory.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "sequence/constantchanneleffect.h"

namespace photon {

/*
 *     QStringList options;
    QString name;
    QString description;
    QVariant defaultValue;
    QByteArray uniqueId;
    ChannelType type;
    */

void ChannelInfo::readFromJson(const QJsonObject &t_json)
{
    name = t_json.value("name").toString();
    description = t_json.value("description").toString();
    uniqueId = t_json.value("uniqueId").toString().toLatin1();
    type = static_cast<ChannelType>(t_json.value("type").toInt());
    defaultValue = t_json.value("name").toVariant();
    if(t_json.contains("options"))
    {
        auto optionArray = t_json.value("options").toArray();

        for(auto option : optionArray)
        {
            options.append(option.toString());
        }
    }
}

void ChannelInfo::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name",name);
    t_json.insert("description",description);
    t_json.insert("uniqueId", QString(uniqueId));
    t_json.insert("type",type);
    t_json.insert("defaultValue",QJsonValue::fromVariant(defaultValue));

    if(!options.isEmpty())
    {
        QJsonArray optionArray;
        for(const auto &option : options)
        {
            optionArray.append(option);
        }
        t_json.insert("options", optionArray);
    }

}

Channel::Channel(const ChannelInfo &t_info, double t_startTime, double t_duration, QObject *t_parent):QObject(t_parent), m_impl(new Impl)
{
    m_impl->info = t_info;
    m_impl->startTime = t_startTime;
    m_impl->duration = t_duration;
    m_impl->uniqueId = t_info.uniqueId;

    m_impl->effects.append(photonApp->plugins()->createChannelEffect(ConstantChannelEffect::info().effectId)); 
    m_impl->effects.back()->m_impl->channel = this;
}

Channel::~Channel()
{
    delete m_impl;
}

Sequence *Channel::sequence() const
{
    QObject *par = parent();

    while(par)
    {
        Sequence *seq = dynamic_cast<Sequence *>(par);
        if(seq)
            return seq;
        par = par->parent();
    }

    return nullptr;
}

void Channel::setDuration(double t_duration)
{
    if(m_impl->duration == t_duration)
        return;
    m_impl->duration = t_duration;
    emit channelUpdated(this);
}

double Channel::duration() const
{
    return m_impl->duration;
}

void Channel::setStartTime(double t_start)
{
    if(m_impl->startTime == t_start)
        return;
    m_impl->startTime = t_start;
    emit channelUpdated(this);
}

double Channel::startTime() const
{
    return m_impl->startTime;
}

void Channel::setEndTime(double t_value)
{
    setDuration(t_value - m_impl->startTime);
}

double Channel::endTime() const
{
    return m_impl->startTime + m_impl->duration;
}

void Channel::updateInfo(const ChannelInfo &t_info)
{
    m_impl->info = t_info;
    m_impl->uniqueId = t_info.uniqueId;
    emit channelUpdated(this);
}

QByteArray Channel::uniqueId() const
{
    return m_impl->uniqueId;
}

ChannelInfo Channel::info() const
{
    return m_impl->info;
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

QColor Channel::processColor(double time)
{
    QColor val = m_impl->info.defaultValue.value<QColor>();

    if(!m_impl->effects.isEmpty())
        val = m_impl->effects.back()->processColor(val, time);

    return val;
}

void Channel::effectUpdated(ChannelEffect *t_effect)
{
    emit effectModified(t_effect);
    emit channelUpdated(this);
}

int Channel::effectCount() const
{
    return m_impl->effects.length();
}

ChannelEffect *Channel::effectAtIndex(int index) const
{
    return m_impl->effects.at(index);
}

void Channel::restore(Project &t_project)
{
    for(auto effect : m_impl->effects)
        effect->restore(t_project);
}

void Channel::readFromJson(const QJsonObject &t_json, const LoadContext &)
{
    m_impl->info.name = t_json.value("name").toString();
    m_impl->info.description = t_json.value("description").toString();
    m_impl->info.type = static_cast<ChannelInfo::ChannelType>(t_json.value("type").toInt());
    m_impl->info.defaultValue = t_json.value("defaultValue");
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();

    for(auto effect : m_impl->effects)
        delete effect;

    m_impl->effects.clear();

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

    t_json.insert("uniqueId",QString::fromLatin1(m_impl->uniqueId));
    t_json.insert("name",m_impl->info.name);
    t_json.insert("description",m_impl->info.description);
    t_json.insert("type",m_impl->info.type);
    t_json.insert("defaultValue",m_impl->info.defaultValue.toJsonValue());

    switch(m_impl->info.type)
    {
    case photon::ChannelInfo::ChannelTypeInteger:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toInt());
        break;
    case photon::ChannelInfo::ChannelTypeBool:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toBool());
        break;
    case photon::ChannelInfo::ChannelTypeNumber:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toDouble());
        break;
    case photon::ChannelInfo::ChannelTypeColor:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toJsonValue());
        break;

    }


}


} // namespace photon
