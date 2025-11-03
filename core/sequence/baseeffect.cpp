#include <QColor>
#include "clipeffect_p.h"
#include "baseeffectparent.h"
#include "clip.h"
#include "channel/parameter/channelparameter.h"
#include "channel/parameter/view/channelparameterwidget.h"

namespace photon {

void BaseEffect::Impl::processContext( BaseEffectEvaluationContext &t_context, double initialRelativeTime)
{

}

QVector<double> BaseEffect::Impl::valuesForChannel(const QByteArray &t_uniqueId, double t_time)
{
    QVector<double> results;

    for(auto channel : channels)
    {
        if(channel->uniqueId() == t_uniqueId)
            return QVector<double>{channel->processValue(t_time).toDouble()};
        if(channel->parentUniqueId() == t_uniqueId)
        {
            results.resize(channel->subChannelIndex() + 1);
            results[channel->subChannelIndex()] = channel->processValue(t_time).toDouble();
        }
    }
    return results;
}

QColor BaseEffect::Impl::colorForChannel(ChannelParameter *t_param, double t_time)
{
    for(auto channel : channels)
    {
        if(channel->uniqueId() == t_param->uniqueId())
        {

            return channel->processValue(t_time).value<QColor>();
        }
    }

    return t_param->value().value<QColor>();
}

BaseEffect::BaseEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->parameters = new ChannelParameterContainer;
}

BaseEffect::~BaseEffect()
{
    delete m_impl->parameters;
    delete m_impl;
}

void BaseEffect::processChannels(ProcessContext &t_context)
{


}

void BaseEffect::addChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->addChannelParameter(t_param);
}

void BaseEffect::removeChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->removeChannelParameter(t_param);
}

BaseEffectParent *BaseEffect::effectParent() const
{
    return m_impl->effectParent;
}

ChannelParameterContainer *BaseEffect::parameters() const
{
    return m_impl->parameters;
}

void BaseEffect::prepareContext(BaseEffectEvaluationContext &t_context) const
{
    t_context.strength = effectParent()->strengthAtTime(t_context.relativeTime);

    t_context.channelValues = m_impl->parameters->valuesFromChannels(m_impl->channels, t_context.relativeTime);



}

double BaseEffect::getNumberAtTime(const QByteArray &t_id, double t_time) const
{
    for(const auto &channel : m_impl->channels)
    {
        if(channel->uniqueId() == t_id)
            return channel->processValue(t_time).toDouble();
    }
    return 0;
}

QColor BaseEffect::getColorAtTime(const QByteArray &t_id, double t_time) const
{
    for(const auto &channel : m_impl->channels)
    {
        if(channel->uniqueId() == t_id)
            return channel->processValue(t_time).value<QColor>();
    }
    return Qt::white;
}

void BaseEffect::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray BaseEffect::id() const
{
    return m_impl->id;
}

QByteArray BaseEffect::uniqueId() const
{
    return m_impl->uniqueId;
}

void BaseEffect::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString BaseEffect::name() const
{
    return m_impl->name;
}

void BaseEffect::updated()
{
    if(m_impl->effectParent)
        m_impl->effectParent->effectUpdated(this);
}

void BaseEffect::startTimeUpdated(double t_value)
{
    for(auto channel : m_impl->channels)
    {
        channel->setStartTime(t_value);
    }
}

void BaseEffect::durationUpdated(double t_value)
{
    for(auto channel : m_impl->channels)
    {
        channel->setDuration(t_value);
    }
}


void BaseEffect::init()
{

}

photon::Channel *BaseEffect::addChannel(const photon::ChannelInfo &t_info, int t_index)
{
    double start = 0.0;
    double duration = 0.0;

    if(m_impl->effectParent)
    {
        start = m_impl->effectParent->startTime();
        duration = m_impl->effectParent->duration();
    }

    auto channel = new Channel(t_info, start, duration, this);
    connect(channel, &Channel::channelUpdated, this, &BaseEffect::channelUpdatedSlot);
    m_impl->channels.append(channel);
    emit channelAdded(channel);
    return channel;
}

void BaseEffect::removeChannel(int t_index)
{
    auto channel = m_impl->channels[t_index];
    m_impl->channels.removeAt(t_index);
    emit channelRemoved(channel);
    delete channel;
}

void BaseEffect::channelUpdatedSlot(Channel *t_channel)
{
    emit channelUpdated(t_channel);
    m_impl->effectParent->markChanged();
}

void BaseEffect::clearChannels()
{
    for(auto channel : m_impl->channels)
    {
        emit channelRemoved(channel);
        delete channel;
    }
    m_impl->channels.clear();
}

const QVector<Channel*> BaseEffect::channels() const
{
    return m_impl->channels;
}

const QVector<Channel*> BaseEffect::channelsForParameter(ChannelParameter *t_param) const
{
    QVector<Channel*> results;
    for(auto channel : channels())
    {
        if(channel->uniqueId() == t_param->uniqueId())
            return QVector<Channel*>{channel};
        if(channel->parentUniqueId() == t_param->uniqueId())
        {
            results.resize(channel->subChannelIndex() + 1);
            results[channel->subChannelIndex()] = channel;
        }
    }
    return results;
}

Channel *BaseEffect::channelAtIndex(int t_index) const
{
    return m_impl->channels[t_index];
}

int BaseEffect::channelCount() const
{
    return m_impl->channels.length();
}

void BaseEffect::addedToParent(BaseEffectParent *t_clip)
{

}

void BaseEffect::layerChanged(Layer *t_layer)
{

}

void BaseEffect::createChannelsFromParameter(ChannelParameter *t_param, ChannelInfo::ChannelType t_type)
{
    auto channels = m_impl->parameters->createChannelsFromParameter(t_param, t_type);

    for(const auto &channelInfo : channels)
        addChannel(channelInfo);
}

QWidget *BaseEffect::createEditor()
{
    auto paramWidget = new ChannelParameterWidget(m_impl->parameters->channelParameters(), [this](ChannelParameter *t_param){
        return channelsForParameter(t_param).isEmpty();
    });
    connect(paramWidget, &ChannelParameterWidget::addChannel,this, &BaseEffect::createChannelsFromParameter);

    return paramWidget;
}

void BaseEffect::restore(Project &)
{

}

void BaseEffect::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString(QUuid::createUuid().toString()).toLatin1();


    for(auto channel : m_impl->channels)
        delete channel;

    m_impl->channels.clear();

    auto channelArray = t_json.value("channels").toArray();
    for(auto channelJson : channelArray)
    {
        Channel *channel = new Channel(ChannelInfo(), m_impl->effectParent->startTime(), m_impl->effectParent->duration(), this);
        connect(channel, &Channel::channelUpdated, this, &BaseEffect::channelUpdatedSlot);
        channel->readFromJson(channelJson.toObject(), t_context);
        m_impl->channels.append(channel);
    }

    m_impl->parameters->readFromJson(t_json, t_context);
}

void BaseEffect::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    QJsonArray array;
    for(auto channel : m_impl->channels)
    {
        QJsonObject channelObj;
        channel->writeToJson(channelObj);
        array.append(channelObj);
    }
    t_json.insert("channels", array);


    m_impl->parameters->writeToJson(t_json);
}


} // namespace photon
