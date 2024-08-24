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
    parentName = t_json.value("parentName").toString();
    description = t_json.value("description").toString();
    uniqueId = t_json.value("uniqueId").toString().toLatin1();
    parentUniqueId = t_json.value("parentUniqueId").toString().toLatin1();
    type = static_cast<ChannelType>(t_json.value("type").toInt());
    defaultValue = t_json.value("name").toVariant();
    subChannelIndex = t_json.value("subChannelIndex").toInt();
    if(t_json.contains("options"))
    {
        auto optionArray = t_json.value("options").toArray();

        for(auto option : optionArray)
        {
            options.append(option.toString());
        }
    }
    if(t_json.contains("subChannelNames"))
    {
        auto optionArray = t_json.value("subChannelNames").toArray();

        for(auto option : optionArray)
        {
            subChannelNames.append(option.toString());
        }
    }
}

void ChannelInfo::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name",name);
    t_json.insert("parentName",parentName);
    t_json.insert("description",description);
    t_json.insert("uniqueId", QString(uniqueId));
    t_json.insert("parentUniqueId", QString(parentUniqueId));
    t_json.insert("type",type);
    t_json.insert("defaultValue",QJsonValue::fromVariant(defaultValue));
    t_json.insert("subChannelIndex",subChannelIndex);

    if(!options.isEmpty())
    {
        QJsonArray optionArray;
        for(const auto &option : options)
        {
            optionArray.append(option);
        }
        t_json.insert("options", optionArray);
    }
    if(!subChannelNames.isEmpty())
    {
        QJsonArray optionArray;
        for(const auto &option : subChannelNames)
        {
            optionArray.append(option);
        }
        t_json.insert("subChannelNames", optionArray);
    }

}

Channel::Channel(const ChannelInfo &t_info, double t_startTime, double t_duration, QObject *t_parent):QObject(t_parent), m_impl(new Impl)
{
    m_impl->info = t_info;
    m_impl->startTime = t_startTime;
    m_impl->duration = t_duration;
    m_impl->uniqueId = t_info.uniqueId;
    m_impl->parentName = t_info.parentName;

    if(t_info.type == ChannelInfo::ChannelTypeColor)
    {
        ChannelInfo subInfo;
        subInfo.parentUniqueId = t_info.uniqueId;
        subInfo.subChannelIndex = 0;
        subInfo.name = t_info.name + ".hue";
        addSubChannel(new Channel{subInfo,t_startTime, t_duration});
        subInfo.name = t_info.name + ".saturation";
        subInfo.subChannelIndex = 1;
        addSubChannel(new Channel{subInfo,t_startTime, t_duration});
        subInfo.name = t_info.name + ".lightness";
        subInfo.subChannelIndex = 2;
        addSubChannel(new Channel{subInfo,t_startTime, t_duration});
        subInfo.name = t_info.name + ".alpha";
        subInfo.subChannelIndex = 3;
        addSubChannel(new Channel{subInfo,t_startTime, t_duration});


    }
    else if(t_info.type == ChannelInfo::ChannelTypePoint)
    {
        ChannelInfo subInfo;
        subInfo.parentUniqueId = t_info.uniqueId;
        subInfo.name = t_info.name + ".x";
        subInfo.subChannelIndex = 0;
        addSubChannel(new Channel{subInfo,t_startTime, t_duration});
        subInfo.name = t_info.name + ".y";
        subInfo.subChannelIndex = 1;
        addSubChannel(new Channel{subInfo,t_startTime, t_duration});


    }
    else
    {
        auto effect = photonApp->plugins()->createChannelEffect(ConstantChannelEffect::info().effectId);
        static_cast<ConstantChannelEffect*>(effect)->setValue(t_info.defaultValue.toDouble());
        m_impl->effects.append(effect);
    }

    if(!m_impl->effects.isEmpty())
        m_impl->effects.back()->m_impl->channel = this;
}

Channel::~Channel()
{
    delete m_impl;
}

void Channel::addSubChannel(Channel *t_channel)
{
    m_impl->subChannels.append(t_channel);
    t_channel->setParent(this);

    connect(t_channel, &Channel::channelUpdated, this, &Channel::channelUpdated);
}

void Channel::removeSubChannel(Channel *t_channel)
{
    m_impl->subChannels.removeOne(t_channel);
    t_channel->setParent(nullptr);
}

const QVector<Channel*> &Channel::subChannels() const
{
    return m_impl->subChannels;
}

int Channel::subChannelCount() const
{
    return m_impl->subChannels.length();
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

void Channel::setParentName(const QString &t_name)
{
    m_impl->parentName = t_name;
}

QString Channel::parentName() const
{
    return m_impl->parentName;
}

void Channel::setDuration(double t_duration)
{
    if(m_impl->duration == t_duration)
        return;
    m_impl->duration = t_duration;


    for(auto subChannel : m_impl->subChannels)
        subChannel->setDuration(t_duration);

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
    for(auto subChannel : m_impl->subChannels)
        subChannel->setStartTime(t_start);

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

QByteArray Channel::parentUniqueId() const
{
    return m_impl->info.parentUniqueId;
}

int Channel::subChannelIndex() const
{
    return m_impl->info.subChannelIndex;
}

ChannelInfo Channel::info() const
{
    return m_impl->info;
}

ChannelInfo::ChannelType Channel::type() const
{
    return m_impl->info.type;
}

void Channel::addEffect(ChannelEffect *t_effect)
{
    t_effect->m_impl->addToChannel(this);
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

QVariant Channel::processValue(double time)
{
    QVariant val = m_impl->info.defaultValue;

    int subChannelCount = m_impl->subChannels.length();
    int valuesSize = std::max(1,subChannelCount);

    float *values = new float[valuesSize];
    if(subChannelCount > 0)
    {
        if(type() == ChannelInfo::ChannelTypePoint && subChannelCount == 2)
        {
            QPointF pt = val.toPointF();
            values[0] = pt.x();
            values[1] = pt.y();
        }
        else if(type() == ChannelInfo::ChannelTypeColor && m_impl->subChannels.length() == 4)
        {
            QColor color = val.value<QColor>();
            color.getHslF(&values[0],&values[1],&values[2],&values[3]);
        }
        else
        {
            for(int i = 0; i < subChannelCount; ++i)
            {
                values[i] = 0;
            }
        }
    }
    else
    {
        values[0] = val.toFloat();
    }



    if(!m_impl->effects.isEmpty())
        values = m_impl->effects.back()->process(values, valuesSize, time);

    if(subChannelCount > 0)
    {
        if(type() == ChannelInfo::ChannelTypePoint && subChannelCount == 2)
        {
            QPointF pt = val.toPointF();
            pt.setX(values[0] + m_impl->subChannels[0]->processValue(time).toFloat());
            pt.setY(values[1] + m_impl->subChannels[1]->processValue(time).toFloat());
            delete[] values;
            return pt;
        }
        else if(type() == ChannelInfo::ChannelTypeColor && m_impl->subChannels.length() == 4)
        {
            QColor color = val.value<QColor>();
            color.setHslF(color.hueF() + m_impl->subChannels[0]->processValue(time).toFloat(),
                color.saturationF() + m_impl->subChannels[1]->processValue(time).toFloat(),
                color.lightnessF() + m_impl->subChannels[2]->processValue(time).toFloat(),
                color.alphaF() + m_impl->subChannels[3]->processValue(time).toFloat());
            delete[] values;
            return color;
        }
    }

    double v = values[0];
    delete[] values;
    return v;
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


    for(auto subChannel : m_impl->subChannels)
        subChannel->restore(t_project);
}

void Channel::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->info.name = t_json.value("name").toString();
    m_impl->info.description = t_json.value("description").toString();
    m_impl->info.type = static_cast<ChannelInfo::ChannelType>(t_json.value("type").toInt());
    m_impl->info.defaultValue = t_json.value("defaultValue");
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    m_impl->info.parentUniqueId = t_json.value("parentUniqueId").toString().toLatin1();
    m_impl->info.subChannelIndex = t_json.value("subChannelIndex").toInt();

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

                effect->m_impl->addToChannel(this);
            }
        }
    }

    if(t_json.contains("subChannels"))
    {
        auto subChannelArray = t_json.value("subChannels").toArray();

        for(auto item : subChannelArray)
        {
            auto channelObj = item.toObject();

            Channel *channel = new Channel(ChannelInfo(), startTime(), duration(), this);
            channel->readFromJson(channelObj, t_context);
            addSubChannel(channel);
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
    t_json.insert("subChannelIndex",m_impl->info.subChannelIndex);
    t_json.insert("parentUniqueId",QString::fromLatin1(m_impl->info.parentUniqueId));
    t_json.insert("defaultValue",m_impl->info.defaultValue.toJsonValue());

    QJsonArray subChannelArray;
    for(auto subChannel : m_impl->subChannels)
    {
        QJsonObject channelJson;
        subChannel->writeToJson(channelJson);
        subChannelArray.append(channelJson);
    }
    t_json.insert("subChannels", subChannelArray);

    switch(m_impl->info.type)
    {
    case photon::ChannelInfo::ChannelTypeIntegerStep:
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
    case photon::ChannelInfo::ChannelTypePoint:
        t_json.insert("defaultValue",m_impl->info.defaultValue.toJsonValue());
        break;

    }


}


} // namespace photon
