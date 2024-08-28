#include <QColor>
#include "channelparametercontainer.h"
#include "channelparameter.h"

namespace photon {

class ChannelParameterContainer::Impl
{
public:
    QColor colorForChannel(ChannelParameter *t_param, double t_time, const QVector<Channel*> &t_channels);
    QVector<double> valuesForChannel(const QByteArray &t_uniqueId, double t_time, const QVector<Channel*> &t_channels);
    QVariant variantForChannel(const QByteArray &t_uniqueId, double t_time, const QVector<Channel*> &t_channels);
    ChannelParameter *findChannelParameter(const QByteArray &);
    QVector<ChannelParameter*> channelParameters;

};



QVector<double> ChannelParameterContainer::Impl::valuesForChannel(const QByteArray &t_uniqueId, double t_time, const QVector<Channel*> &t_channels)
{
    QVector<double> results;

    for(auto channel : t_channels)
    {
        if(channel->uniqueId() == t_uniqueId)
        {
            auto val = channel->processValue(t_time);
            /*
            if(channel->subChannelCount() > 0)
                results.resize(channel->subChannelCount());
            for(auto sub : channel->subChannels())
            {
                results[sub->subChannelIndex()] = sub->processValue(t_time).toDouble();
            }
            if(!results.isEmpty())
                return results;
*/

            if(channel->info().type == ChannelInfo::ChannelTypeColor)
            {

            }
            return QVector<double>{channel->processValue(t_time).toDouble()};
        }
    }
    return results;
}


QVariant ChannelParameterContainer::Impl::variantForChannel(const QByteArray &t_uniqueId, double t_time, const QVector<Channel*> &t_channels)
{

    for(auto channel : t_channels)
    {
        if(channel->uniqueId() == t_uniqueId)
        {
            return channel->processValue(t_time);
        }
    }
    return QVariant{};
}

QColor ChannelParameterContainer::Impl::colorForChannel(ChannelParameter *t_param, double t_time, const QVector<Channel*> &t_channels)
{
    for(auto channel : t_channels)
    {
        if(channel->uniqueId() == t_param->uniqueId())
        {

            return channel->processValue(t_time).value<QColor>();
        }
    }

    return t_param->value().value<QColor>();
}

ChannelParameter *ChannelParameterContainer::Impl::findChannelParameter(const QByteArray &t_uniqueId)
{
    for(auto channelParam : channelParameters)
    {
        if(channelParam->uniqueId() == t_uniqueId)
            return channelParam;
    }
    return nullptr;
}

ChannelParameterContainer::ChannelParameterContainer():m_impl(new Impl)
{

}


ChannelParameterContainer::~ChannelParameterContainer()
{
    delete m_impl;
}

QHash<QByteArray,QVariant> ChannelParameterContainer::valuesFromChannels(const QVector<Channel*> &t_channels, double t_time) const
{
    QHash<QByteArray,QVariant> results;
    for(auto channelParam : m_impl->channelParameters)
    {
        if(channelParam->type() == ChannelParameter::ChannelParameterText)
        {
            results.insert(channelParam->uniqueId(), channelParam->value());
            continue;
        }
        auto values = m_impl->valuesForChannel(channelParam->uniqueId(), t_time, t_channels);

        if(values.length() == channelParam->channels())
        {
            results.insert(channelParam->uniqueId(), channelParam->channelsToVariant(values));
        }
        else
        {
            if(channelParam->type() == ChannelParameter::ChannelParameterColor)
            {
                results.insert(channelParam->uniqueId(), m_impl->colorForChannel(channelParam, t_time,t_channels));
            }
            else
            {
                results.insert(channelParam->uniqueId(), channelParam->value());
            }
        }
    }

    return results;
}

void ChannelParameterContainer::addChannelParameter(ChannelParameter *t_parameter)
{
    m_impl->channelParameters.append(t_parameter);
}

void ChannelParameterContainer::removeChannelParameter(ChannelParameter *t_parameter)
{
    m_impl->channelParameters.removeOne(t_parameter);
}

ChannelParameter *ChannelParameterContainer::channelParameterAtIndex(int t_index) const
{
    return m_impl->channelParameters[t_index];
}

const QVector<ChannelParameter*> ChannelParameterContainer::channelParameters() const
{
    return m_impl->channelParameters;
}

int ChannelParameterContainer::channelParameterCount() const
{
    return m_impl->channelParameters.count();
}

QVector<ChannelInfo> ChannelParameterContainer::createChannelsFromParameter(ChannelParameter *t_param, ChannelInfo::ChannelType t_type)
{
    QVector<ChannelInfo> results;
    if(t_type == ChannelInfo::ChannelTypeNumber)
    {
        if(t_param->channels() == 1)
        {
            ChannelInfo info;
            info.name = t_param->name();
            info.uniqueId = t_param->uniqueId();
            info.type = ChannelInfo::ChannelTypeNumber;
            info.defaultValue = t_param->value();
            results << info;
        }
        else
        {
            int index = 0;
            auto values = t_param->variantToChannels(t_param->value());
            for(const auto &name : t_param->channelNames())
            {
                ChannelInfo info;
                info.name = t_param->name() + "." + name;
                info.uniqueId = t_param->uniqueId() + "." + name.toLatin1();
                info.type = ChannelInfo::ChannelTypeNumber;
                info.defaultValue = values[index];
                info.parentName = t_param->name();
                info.parentUniqueId = t_param->uniqueId();
                info.subChannelIndex = index++;
                results << info;
            }
        }
    }
    else
    {
        ChannelInfo info;
        info.name = t_param->name();
        info.uniqueId = t_param->uniqueId();
        info.type = t_type;
        info.defaultValue = t_param->value();
        results << info;
    }

    return results;

}


void ChannelParameterContainer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{

    auto channelParamArray = t_json.value("channelParameters").toArray();
    for(auto channelJson : channelParamArray)
    {
        auto jsonObj = channelJson.toObject();

        auto channelParam = m_impl->findChannelParameter(jsonObj.value("uniqueId").toString().toLatin1());
        if(channelParam)
            channelParam->readFromJson(jsonObj);
    }
}

void ChannelParameterContainer::writeToJson(QJsonObject &t_json) const
{
    QJsonArray paramArray;
    for(auto channelParam : m_impl->channelParameters)
    {
        QJsonObject channelObj;
        channelParam->writeToJson(channelObj);
        paramArray.append(channelObj);
    }
    t_json.insert("channelParameters", paramArray);
}

} // namespace photon
