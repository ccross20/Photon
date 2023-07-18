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

namespace photon {

Clip::Impl::Impl(Clip *t_facade):facade(t_facade)
{
    uniqueId = QUuid::createUuid().toByteArray();
    easeInCurve.setType(QEasingCurve::Type::InOutCubic);
    easeOutCurve.setType(QEasingCurve::Type::InOutCubic);
    falloffEffects.append(photonApp->plugins()->createFalloffEffect(ConstantFalloffEffect::info().effectId));
    falloffEffects.back()->m_impl->clip = t_facade;
}

void Clip::Impl::setLayer(ClipLayer *t_layer)
{
    if(layer)
        layer->removeClip(facade);
    layer = t_layer;
}

void Clip::Impl::setSequence(Sequence *t_sequence)
{
    sequence = t_sequence;
}

double Clip::Impl::falloff(Fixture *t_fixture)
{
    if(falloffEffects.isEmpty())
        return 0.0;

    return falloffEffects.back()->falloff(t_fixture);
}

void Clip::Impl::markChanged()
{
    if(layer)
        layer->m_impl->notifyClipWasModified(facade);
}

Clip::Clip(QObject *t_parent)
    : QObject{t_parent}, m_impl(new Impl(this))
{
    ChannelInfo info(ChannelInfo::ChannelTypeNumber,"Strength");
    addChannel(info);
}

Clip::Clip(double t_start, double t_duration, QObject *t_parent) : Clip(t_parent)
{
    m_impl->startTime = t_start;
    m_impl->duration = t_duration;
}

Clip::~Clip()
{
    for(auto effect : m_impl->falloffEffects)
        delete effect;
    delete m_impl;
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

double Clip::falloff(Fixture *t_fixture) const
{
    return m_impl->falloff(t_fixture);
}

void Clip::setDefaultFalloff(double t_falloff)
{
    m_impl->defaultFalloff = t_falloff;
    m_impl->markChanged();
    emit clipUpdated(this);
}

double Clip::defaultFalloff() const
{
    return m_impl->defaultFalloff;
}

void Clip::markChanged()
{
    m_impl->markChanged();
}

void Clip::addFalloffEffect(FalloffEffect *t_effect)
{
    if(!m_impl->falloffEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->falloffEffects.back();
    m_impl->falloffEffects.append(t_effect);
    t_effect->m_impl->clip = this;

    emit falloffEffectAdded(t_effect);
    m_impl->markChanged();
}

void Clip::removeFalloffEffect(FalloffEffect *t_effect)
{
    if(m_impl->falloffEffects.removeOne(t_effect))
    {
        t_effect->m_impl->clip = nullptr;
        for(uint i = 1; i < m_impl->falloffEffects.length(); ++i)
        {
            m_impl->falloffEffects[i]->m_impl->previousEffect = m_impl->falloffEffects[i-1];
        }

        emit falloffEffectRemoved(t_effect);
    }
}

FalloffEffect *Clip::falloffEffectAtIndex(int t_index) const
{
    return m_impl->falloffEffects[t_index];
}

void Clip::addMaskEffect(MaskEffect *t_effect)
{
    if(!m_impl->maskEffects.isEmpty())
        t_effect->m_impl->previousEffect = m_impl->maskEffects.back();
    m_impl->maskEffects.append(t_effect);
    t_effect->m_impl->clip = this;

    emit maskAdded(t_effect);
    m_impl->markChanged();
}

void Clip::removeMaskEffect(MaskEffect *t_effect)
{
    if(m_impl->maskEffects.removeOne(t_effect))
    {
        t_effect->m_impl->clip = nullptr;
        for(uint i = 1; i < m_impl->falloffEffects.length(); ++i)
        {
            m_impl->maskEffects[i]->m_impl->previousEffect = m_impl->maskEffects[i-1];
        }

        emit maskRemoved(t_effect);
    }
}

MaskEffect *Clip::maskEffectAtIndex(int index) const
{
    return m_impl->maskEffects[index];
}

int Clip::maskEffectCount() const
{
    return m_impl->maskEffects.length();
}

const QVector<Fixture*> Clip::maskedFixtures() const
{
    if(m_impl->maskEffects.isEmpty())
        return photonApp->project()->fixtures()->fixtures();
    return m_impl->maskEffects.back()->process(photonApp->project()->fixtures()->fixtures());
}

int Clip::falloffEffectCount() const
{
    return m_impl->falloffEffects.length();
}

bool Clip::timeIsValid(double t_time) const
{
    return m_impl->startTime < t_time && t_time < endTime() + m_impl->defaultFalloff;
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

void Clip::maskUpdatedSlot(photon::MaskEffect *t_mask)
{
    emit maskUpdated(t_mask);
    m_impl->markChanged();
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

void Clip::falloffUpdatedSlot(photon::FalloffEffect *t_falloff)
{
    emit falloffUpdated(t_falloff);
    m_impl->markChanged();
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

QWidget *Clip::widget() const
{
    return nullptr;
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

void Clip::addClipEffect(ClipEffect *t_effect)
{
    m_impl->clipEffects.append(t_effect);
    t_effect->m_impl->clip = this;

    emit clipEffectAdded(t_effect);
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
    for(auto effect : m_impl->falloffEffects)
        delete effect;
    m_impl->falloffEffects.clear();

    m_impl->type = t_json.value("type").toString().toLatin1();
    m_impl->startTime = t_json.value("startTime").toDouble();
    m_impl->duration = t_json.value("duration").toDouble();
    m_impl->defaultFalloff = t_json.value("defaultFalloff").toDouble();
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
                m_impl->clipEffects.append(effect);

            }
            else
            {
                qWarning() << "Could not find clip effect:" << id;
            }
        }
    }

    if(t_json.contains("falloffEffects"))
    {
        auto effectArray = t_json.value("falloffEffects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createFalloffEffect(id.toLatin1());

            if(effect){
                effect->readFromJson(effectObj);
                if(!m_impl->falloffEffects.isEmpty())
                    effect->m_impl->previousEffect = m_impl->falloffEffects.back();
                m_impl->falloffEffects.append(effect);

                effect->m_impl->clip = this;
            }
        }
    }

    if(t_json.contains("maskEffects"))
    {
        auto effectArray = t_json.value("maskEffects").toArray();

        for(auto item : effectArray)
        {
            auto effectObj = item.toObject();
            auto id = effectObj.value("id").toString();

            auto effect = photonApp->plugins()->createMaskEffect(id.toLatin1());

            if(effect){
                effect->readFromJson(effectObj);
                if(!m_impl->maskEffects.isEmpty())
                    effect->m_impl->previousEffect = m_impl->maskEffects.back();
                m_impl->maskEffects.append(effect);

                effect->m_impl->clip = this;
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

    qDebug() << "Load clip" << name();

}

void Clip::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("type", QString(m_impl->type));
    t_json.insert("startTime", m_impl->startTime);
    t_json.insert("duration", m_impl->duration);
    t_json.insert("defaultFalloff", m_impl->defaultFalloff);
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

    QJsonArray falloffArray;
    for(auto effect : m_impl->falloffEffects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        falloffArray.append(effectObj);
    }
    t_json.insert("falloffEffects", falloffArray);


    QJsonArray maskArray;
    for(auto effect : m_impl->maskEffects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        maskArray.append(effectObj);
    }
    t_json.insert("maskEffects", maskArray);

    QJsonArray array;
    for(auto channel : m_impl->channels)
    {
        QJsonObject channelObj;
        channel->writeToJson(channelObj);
        array.append(channelObj);
    }
    t_json.insert("channels", array);
}

} // namespace photon
