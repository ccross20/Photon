#include <QColor>
#include "clipeffect_p.h"
#include "clip.h"
#include "channel/parameter/channelparameter.h"
#include "channel/parameter/view/channelparameterwidget.h"

namespace photon {

void ClipEffect::Impl::processContext( ClipEffectEvaluationContext &t_context, double initialRelativeTime)
{

}

QVector<double> ClipEffect::Impl::valuesForChannel(const QByteArray &t_uniqueId, double t_time)
{
    QVector<double> results;

    for(auto channel : channels)
    {
        if(channel->uniqueId() == t_uniqueId)
            return QVector<double>{channel->processDouble(t_time)};
        if(channel->parentUniqueId() == t_uniqueId)
        {
            results.resize(channel->subChannelIndex() + 1);
            results[channel->subChannelIndex()] = channel->processDouble(t_time);
        }
    }
    return results;
}

QColor ClipEffect::Impl::colorForChannel(ChannelParameter *t_param, double t_time)
{
    for(auto channel : channels)
    {
        if(channel->uniqueId() == t_param->uniqueId())
        {

            return channel->processColor(t_time);
        }
    }

    return t_param->value().value<QColor>();
}

ClipEffect::ClipEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->parameters = new ChannelParameterContainer;
}

ClipEffect::~ClipEffect()
{
    delete m_impl->parameters;
    delete m_impl;
}

void ClipEffect::processChannels(ProcessContext &t_context)
{


}

void ClipEffect::addChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->addChannelParameter(t_param);
}

void ClipEffect::removeChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->removeChannelParameter(t_param);
}

ChannelParameterContainer *ClipEffect::parameters() const
{
    return m_impl->parameters;
}

void ClipEffect::prepareContext(ClipEffectEvaluationContext &t_context) const
{
    t_context.strength = clip()->strengthAtTime(t_context.relativeTime);

    t_context.channelValues = m_impl->parameters->valuesFromChannels(m_impl->channels, t_context.relativeTime);


}

double ClipEffect::getNumberAtTime(const QByteArray &t_id, double t_time) const
{
    for(const auto &channel : m_impl->channels)
    {
        if(channel->uniqueId() == t_id)
            return channel->processDouble(t_time);
    }
    return 0;
}

QColor ClipEffect::getColorAtTime(const QByteArray &t_id, double t_time) const
{
    for(const auto &channel : m_impl->channels)
    {
        if(channel->uniqueId() == t_id)
            return channel->processColor(t_time);
    }
    return Qt::white;
}

Clip *ClipEffect::clip() const
{
    return m_impl->clip;
}

void ClipEffect::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray ClipEffect::id() const
{
    return m_impl->id;
}

QByteArray ClipEffect::uniqueId() const
{
    return m_impl->uniqueId;
}

void ClipEffect::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString ClipEffect::name() const
{
    return m_impl->name;
}

void ClipEffect::updated()
{
    if(m_impl->clip)
        m_impl->clip->clipEffectUpdatedSlot(this);
}

void ClipEffect::startTimeUpdated(double t_value)
{
    for(auto channel : m_impl->channels)
    {
        channel->setStartTime(t_value);
    }
}

void ClipEffect::durationUpdated(double t_value)
{
    for(auto channel : m_impl->channels)
    {
        channel->setDuration(t_value);
    }
}


void ClipEffect::init()
{

}

photon::Channel *ClipEffect::addChannel(const photon::ChannelInfo &t_info, int t_index)
{
    double start = 0.0;
    double duration = 0.0;

    if(m_impl->clip)
    {
        start = m_impl->clip->startTime();
        duration = m_impl->clip->duration();
    }

    auto channel = new Channel(t_info, start, duration, this);
    connect(channel, &Channel::channelUpdated, this, &ClipEffect::channelUpdatedSlot);
    m_impl->channels.append(channel);
    emit channelAdded(channel);
    return channel;
}

void ClipEffect::removeChannel(int t_index)
{
    auto channel = m_impl->channels[t_index];
    m_impl->channels.removeAt(t_index);
    emit channelRemoved(channel);
    delete channel;
}

void ClipEffect::channelUpdatedSlot(Channel *t_channel)
{
    emit channelUpdated(t_channel);
    m_impl->clip->markChanged();
}

void ClipEffect::clearChannels()
{
    for(auto channel : m_impl->channels)
    {
        emit channelRemoved(channel);
        delete channel;
    }
    m_impl->channels.clear();
}

const QVector<Channel*> ClipEffect::channels() const
{
    return m_impl->channels;
}

const QVector<Channel*> ClipEffect::channelsForParameter(ChannelParameter *t_param) const
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

Channel *ClipEffect::channelAtIndex(int t_index) const
{
    return m_impl->channels[t_index];
}

int ClipEffect::channelCount() const
{
    return m_impl->channels.length();
}

void ClipEffect::addedToClip(Clip *t_clip)
{

}

void ClipEffect::layerChanged(Layer *t_layer)
{

}

void ClipEffect::createChannelsFromParameter(ChannelParameter *t_param, ChannelInfo::ChannelType t_type)
{
    auto channels = m_impl->parameters->createChannelsFromParameter(t_param, t_type);

    for(const auto &channelInfo : channels)
        addChannel(channelInfo);
}

QWidget *ClipEffect::createEditor()
{
    auto paramWidget = new ChannelParameterWidget(m_impl->parameters->channelParameters(), [this](ChannelParameter *t_param){
        return channelsForParameter(t_param).isEmpty();
    });
    connect(paramWidget, &ChannelParameterWidget::addChannel,this, &ClipEffect::createChannelsFromParameter);

    return paramWidget;
}

void ClipEffect::restore(Project &)
{

}

void ClipEffect::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
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
        Channel *channel = new Channel(ChannelInfo(), m_impl->clip->startTime(), m_impl->clip->duration(), this);
        connect(channel, &Channel::channelUpdated, this, &ClipEffect::channelUpdatedSlot);
        channel->readFromJson(channelJson.toObject(), t_context);
        m_impl->channels.append(channel);
    }

    m_impl->parameters->readFromJson(t_json, t_context);
}

void ClipEffect::writeToJson(QJsonObject &t_json) const
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
