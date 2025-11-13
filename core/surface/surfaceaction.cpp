#include <QJsonArray>
#include "surfaceaction_p.h"
#include "project/project.h"
#include "sequence/clipeffect_p.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "channel/parameter/view/channelparameterwidget.h"
#include "channel/parameter/channelparametercontainer.h"
#include "channel/parameter/channelparameter.h"
#include "viewer/actionsettingswidget.h"
#include "surfacegizmocontainer.h"

namespace photon {

SurfaceAction::Impl::Impl(SurfaceAction *t_facade):facade(t_facade)
{
    parameters = new ChannelParameterContainer;
    uniqueId = QUuid::createUuid().toByteArray();
}

SurfaceAction::SurfaceAction(QObject *t_parent): QObject{t_parent}, m_impl(new Impl(this))
{
    ChannelInfo info(ChannelInfo::ChannelTypeNumber,"Strength","Strength",1.0);
    addChannel(info);
}

SurfaceAction::~SurfaceAction()
{
    delete m_impl->parameters;
    delete m_impl;
}

SurfaceGizmoContainer *SurfaceAction::parentContainer() const
{
    return static_cast<SurfaceGizmoContainer*>(parent());
}

QByteArray SurfaceAction::enableOnGizmoId() const
{
    return m_impl->enableOnGizmo;
}

void SurfaceAction::setEnableOnGizmoId(const QByteArray &t_gizmoId)
{
    m_impl->enableOnGizmo = t_gizmoId;
}

QByteArray SurfaceAction::type() const
{
    return m_impl->type;
}

QByteArray SurfaceAction::uniqueId() const
{
    return m_impl->uniqueId;
}

void SurfaceAction::setName(const QString &t_value)
{
    m_impl->name = t_value;
}

void SurfaceAction::setId(const QByteArray &t_value)
{
    m_impl->id = t_value;
}

QByteArray SurfaceAction::id() const
{
    return m_impl->id;
}

void SurfaceAction::setType(const QByteArray t_type)
{
    m_impl->type = t_type;
}

QString SurfaceAction::name() const
{
    return m_impl->name;
}



void SurfaceAction::markChanged()
{

}

ChannelParameterContainer *SurfaceAction::parameters() const
{
    return m_impl->parameters;
}

void SurfaceAction::processChannels(ProcessContext &t_context)
{

    for(auto effect : m_impl->clipEffects)
    {
        effect->processChannels(t_context);
    }
}

double SurfaceAction::strengthAtTime(double t_value) const
{
    double strengthValue = channelAtIndex(0)->processValue(t_value).toDouble() * m_impl->strength;

    return strengthValue;
}

double SurfaceAction::startTime() const
{
    return 0.0;
}


double SurfaceAction::duration() const
{
    return m_impl->duration;
}

void SurfaceAction::setDuration(double t_value)
{
    m_impl->duration = t_value;
}

void SurfaceAction::restore(Project &t_project)
{
    for(auto channel : m_impl->channels)
        channel->restore(t_project);

    for(auto effect : m_impl->clipEffects)
        effect->restore(t_project);
}

void SurfaceAction::clearChannels()
{
    for(auto channel : m_impl->channels)
    {
        emit channelRemoved(channel);
        delete channel;
    }
    m_impl->channels.clear();
}

const QVector<Channel*> SurfaceAction::channels() const
{
    return m_impl->channels;
}

Channel *SurfaceAction::channelAtIndex(int t_index) const
{
    return m_impl->channels[t_index];
}

int SurfaceAction::channelCount() const
{
    return m_impl->channels.length();
}

double SurfaceAction::strength() const
{
    return m_impl->strength;
}

void SurfaceAction::setStrength(double t_value)
{
    m_impl->strength = std::max(std::min(t_value, 1.0),0.0);
    emit actionUpdated(this);
}


const QVector<Channel*> SurfaceAction::channelsForParameter(ChannelParameter *t_param) const
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

QWidget *SurfaceAction::widget() const
{
    if(m_impl->parameters->channelParameterCount() == 0)
        return nullptr;

    auto paramWidget = new ChannelParameterWidget(m_impl->parameters->channelParameters(), [this](ChannelParameter *t_param){
        return channelsForParameter(t_param).isEmpty();
    });
    connect(paramWidget, &ChannelParameterWidget::addChannel,this, &SurfaceAction::createChannelsFromParameter);

    return paramWidget;
}

QWidget *SurfaceAction::settingsWidget() const
{
    return new ActionSettingsWidget(const_cast<SurfaceAction*>(this));
}

void SurfaceAction::createChannelsFromParameter(ChannelParameter *t_param, ChannelInfo::ChannelType t_type)
{
    auto channels = m_impl->parameters->createChannelsFromParameter(t_param, t_type);

    for(const auto &channelInfo : channels)
        addChannel(channelInfo);
}

void SurfaceAction::addChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->addChannelParameter(t_param);
}

void SurfaceAction::removeChannelParameter(ChannelParameter *t_param)
{
    m_impl->parameters->removeChannelParameter(t_param);
}

photon::Channel *SurfaceAction::addChannel(const photon::ChannelInfo &t_info, int t_index)
{
    auto channel = new Channel(t_info, startTime(), duration(), this);
    connect(channel, &Channel::channelUpdated, this, &SurfaceAction::channelUpdatedSlot);
    m_impl->channels.append(channel);
    emit channelAdded(channel);
    return channel;
}

void SurfaceAction::removeChannel(int t_index)
{
    auto channel = m_impl->channels[t_index];
    m_impl->channels.removeAt(t_index);
    emit channelRemoved(channel);
    delete channel;
}

void SurfaceAction::channelUpdatedSlot(Channel *t_channel)
{
    emit channelUpdated(t_channel);
    markChanged();
}


void SurfaceAction::effectUpdated(photon::BaseEffect *t_effect)
{
    effectUpdatedSlot(t_effect);
}

void SurfaceAction::effectUpdatedSlot(photon::BaseEffect *t_effect)
{
    emit clipEffectUpdated(t_effect);
    markChanged();
}

const QVector<BaseEffect*> &SurfaceAction::clipEffects() const
{
    return m_impl->clipEffects;
}

void SurfaceAction::addClipEffect(BaseEffect *t_effect)
{
    m_impl->clipEffects.append(t_effect);
    t_effect->setParent(this);
    t_effect->m_impl->effectParent = this;

    emit clipEffectAdded(t_effect);
    t_effect->addedToParent(this);
}

void SurfaceAction::removeClipEffect(BaseEffect *t_effect)
{
    if(m_impl->clipEffects.removeOne(t_effect))
    {
        t_effect->m_impl->effectParent = nullptr;

        emit clipEffectRemoved(t_effect);
    }
}

BaseEffect *SurfaceAction::clipEffectAtIndex(int t_index) const
{
    return m_impl->clipEffects[t_index];
}

int SurfaceAction::clipEffectCount() const
{
    return m_impl->clipEffects.length();
}

void SurfaceAction::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{

    m_impl->type = t_json.value("type").toString().toLatin1();
    m_impl->strength = t_json.value("strength").toDouble(1.0);
    m_impl->duration = t_json.value("duration").toDouble(0.0);
    m_impl->uniqueId = t_json.value("uniqueId").toString(QUuid::createUuid().toString()).toLatin1();
    m_impl->name = t_json.value("name").toString();
    m_impl->id = t_json.value("id").toString().toLatin1();
    m_impl->enableOnGizmo = t_json.value("enableOnGizmo").toString().toLatin1();

    if(t_json.contains("clipEffects"))
    {
        auto effectArray = t_json.value("clipEffects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createClipEffect(id.toLatin1());

            if(effect){
                effect->m_impl->effectParent = this;
                effect->readFromJson(effectObj, t_context);
                effect->setParent(this);
                m_impl->clipEffects.append(effect);
                effect->addedToParent(this);

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
        connect(channel, &Channel::channelUpdated, this, &SurfaceAction::channelUpdatedSlot);
        channel->readFromJson(channelJson.toObject(), t_context);
        m_impl->channels.append(channel);
    }

    m_impl->parameters->readFromJson(t_json, t_context);

}

void SurfaceAction::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("type", QString(m_impl->type));
    t_json.insert("strength", m_impl->strength);
    t_json.insert("duration", m_impl->duration);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    t_json.insert("id", QString(m_impl->id));
    t_json.insert("name", QString(m_impl->name));
    t_json.insert("enableOnGizmo", QString(m_impl->enableOnGizmo));

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
