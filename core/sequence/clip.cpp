#include <QJsonArray>
#include "clip_p.h"
#include "cliplayer_p.h"
#include "falloff/falloffeffect_p.h"
#include "falloff/constantfalloffeffect.h"
#include "fixture/maskeffect_p.h"
#include "sequence.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "clipeffect_p.h"
#include "channel/parameter/view/channelparameterwidget.h"
#include "channel/parameter/channelparametercontainer.h"
#include "channel/parameter/channelparameter.h"

namespace photon {

Clip::Impl::Impl(Clip *t_facade):facade(t_facade)
{
    parameters = new ChannelParameterContainer;
    uniqueId = QUuid::createUuid().toByteArray();
    easeInCurve.setType(QEasingCurve::Type::InOutCubic);
    easeOutCurve.setType(QEasingCurve::Type::InOutCubic);
}

void Clip::Impl::setLayer(ClipLayer *t_layer)
{
    if(layer)
        layer->removeClip(facade);
    layer = t_layer;
    facade->layerDidChange(layer);
}

void Clip::Impl::setSequence(Sequence *t_sequence)
{
    sequence = t_sequence;
}

void Clip::Impl::markChanged()
{
    if(layer)
        layer->m_impl->notifyClipWasModified(facade);
}

Clip::Clip(QObject *t_parent)
    : QObject{t_parent}, m_impl(new Impl(this))
{
    ChannelInfo info(ChannelInfo::ChannelTypeNumber,"Strength","Strength",1.0);
    addChannel(info);
}

Clip::Clip(double t_start, double t_duration, QObject *t_parent) : Clip(t_parent)
{
    m_impl->startTime = t_start;
    m_impl->duration = t_duration;
}

Clip::~Clip()
{
    delete m_impl->parameters;
    delete m_impl;
}

void Clip::layerDidChange(Layer *t_layer)
{
    for(auto effect : m_impl->clipEffects)
        effect->layerChanged(t_layer);
}

QByteArray Clip::type() const
{
    return m_impl->type;
}

QByteArray Clip::uniqueId() const
{
    return m_impl->uniqueId;
}

void Clip::setName(const QString &t_value)
{
    m_impl->name = t_value;
}

void Clip::setId(const QByteArray &t_value)
{
    m_impl->id = t_value;
}

QByteArray Clip::id() const
{
    return m_impl->id;
}

void Clip::setType(const QByteArray t_type)
{
    m_impl->type = t_type;
}

QString Clip::name() const
{
    return m_impl->name;
}

Sequence *Clip::sequence() const
{
    return m_impl->sequence;
}

ClipLayer *Clip::layer() const
{
    return m_impl->layer;
}

void Clip::markChanged()
{
    m_impl->markChanged();
}

ChannelParameterContainer *Clip::parameters() const
{
    return m_impl->parameters;
}

bool Clip::timeIsValid(double t_time) const
{
    return m_impl->startTime < t_time && t_time < endTime();
}

void Clip::processChannels(ProcessContext &t_context)
{


    for(auto effect : m_impl->clipEffects)
    {
        effect->processChannels(t_context);
    }
}

double Clip::strengthAtTime(double t_value) const
{
    double outStart = duration() - m_impl->easeOutDuration;
    double strengthValue = channelAtIndex(0)->processDouble(t_value) * m_impl->strength;

    if(t_value > outStart)
    {
        return m_impl->easeOutCurve.valueForProgress(1 - ((t_value - outStart) / m_impl->easeOutDuration)) * strengthValue;
    }

    if(t_value < m_impl->easeInDuration)
    {
        return m_impl->easeInCurve.valueForProgress(t_value / m_impl->easeInDuration) * strengthValue;
    }

    return strengthValue;
}

void Clip::setStartTime(double t_value)
{
    if(m_impl->startTime == t_value)
        return;
    m_impl->startTime = t_value;

    startTimeUpdated(t_value);
    m_impl->markChanged();
    emit clipUpdated(this);
}

void Clip::setEndTime(double t_value)
{
    setDuration(t_value - m_impl->startTime);
}

void Clip::setDuration(double t_value)
{
    if(m_impl->duration == t_value)
        return;
    m_impl->duration = t_value;

    durationUpdated(t_value);
    m_impl->markChanged();
    emit clipUpdated(this);
}

double Clip::startTime() const
{
    return m_impl->startTime;
}

double Clip::endTime() const
{
    return m_impl->startTime + m_impl->duration;
}

double Clip::duration() const
{
    return m_impl->duration;
}

void Clip::startTimeUpdated(double t_value)
{
    for(auto channel : m_impl->channels)
    {
        channel->setStartTime(t_value);
    }

    for(auto effect : m_impl->clipEffects)
        effect->startTimeUpdated(t_value);
}

void Clip::durationUpdated(double t_value)
{
    for(auto channel : m_impl->channels)
    {
        channel->setDuration(t_value);
    }
    for(auto effect : m_impl->clipEffects)
        effect->durationUpdated(t_value);
}

void Clip::restore(Project &t_project)
{
    for(auto channel : m_impl->channels)
        channel->restore(t_project);
}

void Clip::clearChannels()
{
    for(auto channel : m_impl->channels)
    {
        emit channelRemoved(channel);
        delete channel;
    }
    m_impl->channels.clear();
}

const QVector<Channel*> Clip::channels() const
{
    return m_impl->channels;
}

Channel *Clip::channelAtIndex(int t_index) const
{
    return m_impl->channels[t_index];
}

int Clip::channelCount() const
{
    return m_impl->channels.length();
}

double Clip::strength() const
{
    return m_impl->strength;
}

double Clip::easeInDuration() const
{
    return m_impl->easeInDuration;
}

double Clip::easeOutDuration() const
{
    return m_impl->easeOutDuration;
}

QEasingCurve::Type Clip::easeInType() const
{
    return m_impl->easeInCurve.type();
}

QEasingCurve::Type Clip::easeOutType() const
{
    return m_impl->easeOutCurve.type();
}

void Clip::setStrength(double t_value)
{
    m_impl->strength = std::max(std::min(t_value, 1.0),0.0);
    m_impl->markChanged();
    emit clipUpdated(this);
}

void Clip::setEaseInDuration(double t_value)
{
    m_impl->easeInDuration = std::min(std::max(t_value,0.0), m_impl->duration);

    if(m_impl->duration - m_impl->easeOutDuration < m_impl->easeInDuration)
        m_impl->easeOutDuration = m_impl->duration - m_impl->easeInDuration;
    m_impl->markChanged();
    emit clipUpdated(this);
}

void Clip::setEaseOutDuration(double t_value)
{
    m_impl->easeOutDuration = std::min(std::max(t_value,0.0), m_impl->duration);
    if(m_impl->duration - m_impl->easeOutDuration < m_impl->easeInDuration)
        m_impl->easeInDuration = m_impl->duration - m_impl->easeOutDuration;
    m_impl->markChanged();
    emit clipUpdated(this);
}

void Clip::setEaseInType(QEasingCurve::Type t_value)
{
    m_impl->easeInCurve.setType(t_value);
    m_impl->markChanged();
    emit clipUpdated(this);
}

void Clip::setEaseOutType(QEasingCurve::Type t_value)
{
    m_impl->easeOutCurve.setType(t_value);
    m_impl->markChanged();
    emit clipUpdated(this);
}

const QVector<Channel*> Clip::channelsForParameter(ChannelParameter *t_param) const
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

QWidget *Clip::widget() const
{
    if(m_impl->parameters->channelParameterCount() == 0)
        return nullptr;

    auto paramWidget = new ChannelParameterWidget(m_impl->parameters->channelParameters(), [this](ChannelParameter *t_param){
        return channelsForParameter(t_param).isEmpty();
    });
    connect(paramWidget, &ChannelParameterWidget::addChannel,this, &Clip::createChannelsFromParameter);

    return paramWidget;
}

void Clip::createChannelsFromParameter(ChannelParameter *t_param, ChannelInfo::ChannelType t_type)
{
    auto channels = m_impl->parameters->createChannelsFromParameter(t_param, t_type);

    for(const auto &channelInfo : channels)
        addChannel(channelInfo);
}

void Clip::addChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->addChannelParameter(t_param);
}

void Clip::removeChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->removeChannelParameter(t_param);
}

photon::Channel *Clip::addChannel(const photon::ChannelInfo &t_info, int t_index)
{
    auto channel = new Channel(t_info, startTime(), duration(), this);
    connect(channel, &Channel::channelUpdated, this, &Clip::channelUpdatedSlot);
    m_impl->channels.append(channel);
    emit channelAdded(channel);
    return channel;
}

void Clip::removeChannel(int t_index)
{
    auto channel = m_impl->channels[t_index];
    m_impl->channels.removeAt(t_index);
    emit channelRemoved(channel);
    delete channel;
}

void Clip::channelUpdatedSlot(Channel *t_channel)
{
    emit channelUpdated(t_channel);
    markChanged();
}

void Clip::clipEffectUpdatedSlot(photon::ClipEffect *t_effect)
{
    emit clipEffectUpdated(t_effect);
    markChanged();
}

const QVector<ClipEffect*> &Clip::clipEffects() const
{
    return m_impl->clipEffects;
}

void Clip::addClipEffect(ClipEffect *t_effect)
{
    m_impl->clipEffects.append(t_effect);
    t_effect->setParent(this);
    t_effect->m_impl->clip = this;

    emit clipEffectAdded(t_effect);
    t_effect->addedToClip(this);
    if(layer())
        t_effect->layerChanged(layer());
    m_impl->markChanged();
}

void Clip::removeClipEffect(ClipEffect *t_effect)
{
    if(m_impl->clipEffects.removeOne(t_effect))
    {
        t_effect->m_impl->clip = nullptr;

        emit clipEffectRemoved(t_effect);
    }
}

ClipEffect *Clip::clipEffectAtIndex(int t_index) const
{
    return m_impl->clipEffects[t_index];
}

int Clip::clipEffectCount() const
{
    return m_impl->clipEffects.length();
}

void Clip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{

    m_impl->type = t_json.value("type").toString().toLatin1();
    m_impl->startTime = t_json.value("startTime").toDouble();
    m_impl->duration = t_json.value("duration").toDouble();
    m_impl->easeInCurve.setType(static_cast<QEasingCurve::Type>(t_json.value("easeInType").toInt(QEasingCurve::Type::InOutCubic)));
    m_impl->easeOutCurve.setType(static_cast<QEasingCurve::Type>(t_json.value("easeOutType").toInt(QEasingCurve::Type::InOutCubic)));
    m_impl->easeInDuration = t_json.value("easeInDuration").toDouble();
    m_impl->easeOutDuration = t_json.value("easeOutDuration").toDouble();
    m_impl->strength = t_json.value("strength").toDouble(1.0);
    m_impl->uniqueId = t_json.value("uniqueId").toString(QUuid::createUuid().toString()).toLatin1();
    m_impl->name = t_json.value("name").toString();
    m_impl->id = t_json.value("id").toString().toLatin1();

    if(t_json.contains("clipEffects"))
    {
        auto effectArray = t_json.value("clipEffects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createClipEffect(id.toLatin1());

            if(effect){
                effect->m_impl->clip = this;
                effect->readFromJson(effectObj, t_context);
                effect->setParent(this);
                m_impl->clipEffects.append(effect);
                effect->addedToClip(this);

            }
            else
            {
                qWarning() << "Could not find clip effect:" << id;
            }
        }
    }


    for(auto channel : m_impl->channels)
        delete channel;

    m_impl->channels.clear();

    auto channelArray = t_json.value("channels").toArray();
    for(auto channelJson : channelArray)
    {
        Channel *channel = new Channel(ChannelInfo(), startTime(), duration(), this);
        connect(channel, &Channel::channelUpdated, this, &Clip::channelUpdatedSlot);
        channel->readFromJson(channelJson.toObject(), t_context);
        m_impl->channels.append(channel);
    }

    m_impl->parameters->readFromJson(t_json, t_context);

}

void Clip::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("type", QString(m_impl->type));
    t_json.insert("startTime", m_impl->startTime);
    t_json.insert("duration", m_impl->duration);
    t_json.insert("easeInType", m_impl->easeInCurve.type());
    t_json.insert("easeOutType", m_impl->easeOutCurve.type());
    t_json.insert("easeInDuration", m_impl->easeInDuration);
    t_json.insert("easeOutDuration", m_impl->easeOutDuration);
    t_json.insert("strength", m_impl->strength);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    t_json.insert("id", QString(m_impl->id));
    t_json.insert("name", QString(m_impl->name));

    QJsonArray clipEffectArray;
    for(auto effect : m_impl->clipEffects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        clipEffectArray.append(effectObj);
    }
    t_json.insert("clipEffects", clipEffectArray);

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
