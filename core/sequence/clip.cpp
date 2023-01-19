#include <QJsonArray>
#include "clip_p.h"
#include "cliplayer_p.h"
#include "falloff/falloffeffect_p.h"
#include "falloff/constantfalloffeffect.h"
#include "fixture/fixturemask.h"
#include "routine/routine.h"
#include "sequence.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routinecollection.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

Clip::Impl::Impl(Clip *t_facade):facade(t_facade)
{
    falloffEffects.append(photonApp->plugins()->createFalloffEffect(ConstantFalloffEffect::info().effectId));
    falloffEffects.back()->m_impl->clip = t_facade;
}

void Clip::Impl::setLayer(ClipLayer *t_layer)
{
    layer = t_layer;
}

void Clip::Impl::setSequence(Sequence *t_sequence)
{
    sequence = t_sequence;
}

void Clip::Impl::rebuildFalloffCache()
{
    for(auto fixture : sequence->project()->fixtures()->fixtures())
    {
        double value = 0.0;

        if(!falloffEffects.isEmpty())
            value = falloffEffects.back()->falloff(fixture);

        cachedFalloffs.insert(fixture, value);
    }
}

double Clip::Impl::falloff(Fixture *t_fixture)
{
    if(falloffEffects.isEmpty())
        return 0.0;

    return cachedFalloffs.value(t_fixture, 0.0);
}

void Clip::Impl::markChanged()
{
    if(layer)
        layer->m_impl->notifyClipWasModified(facade);

    rebuildFalloffCache();
}

Clip::Clip(QObject *t_parent)
    : QObject{t_parent}, m_impl(new Impl(this))
{

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

QString Clip::name() const
{
    if(m_impl->routine)
        return m_impl->routine->name();
    else
        return "[Empty]";
}

Sequence *Clip::sequence() const
{
    return m_impl->sequence;
}

ClipLayer *Clip::layer() const
{
    return m_impl->layer;
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

        m_impl->rebuildFalloffCache();
        emit falloffEffectRemoved(t_effect);
    }
}

FalloffEffect *Clip::falloffEffectAtIndex(int index) const
{
    return m_impl->falloffEffects[index];
}

int Clip::falloffEffectCount() const
{
    return m_impl->falloffEffects.length();
}


Channel *Clip::channelAtIndex(int index) const
{
    return m_impl->channels.at(index);
}

int Clip::channelCount() const
{
    return m_impl->channels.length();
}

void Clip::processChannels(ProcessContext &t_context)
{
    if(!m_impl->routine)
        return;

    double initialRelativeTime = t_context.globalTime - startTime();

    RoutineEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.fixture = t_context.fixture;
    localContext.canvasImage = t_context.canvasImage;


    if(!m_impl->mask)
    {
        for(auto fixture : sequence()->project()->fixtures()->fixtures())
        {            
            localContext.relativeTime = initialRelativeTime - m_impl->falloff(fixture);
            localContext.fixture = fixture;

            localContext.channelValues.clear();
            for(const auto &channel : m_impl->channels)
            {
                localContext.channelValues.append(channel->processDouble(localContext.relativeTime));
            }

            if(localContext.channelValues.length() > 0)
                localContext.strength = localContext.channelValues[0].toDouble();



            m_impl->routine->evaluate(&localContext);
            m_impl->routine->markClean();
        }
    }
    else
    {
        for(auto fixture : m_impl->mask->fixtures())
        {
            localContext.relativeTime = initialRelativeTime - m_impl->falloff(fixture);
            localContext.fixture = fixture;

            localContext.channelValues.clear();
            for(const auto &channel : m_impl->channels)
            {
                localContext.channelValues.append(channel->processDouble(localContext.relativeTime));
            }

            if(localContext.channelValues.length() > 0)
                localContext.strength = localContext.channelValues[0].toDouble();


            m_impl->routine->evaluate(&localContext);
            m_impl->routine->markClean();
        }
    }



}

void Clip::setRoutine(Routine *t_routine)
{
    if(m_impl->routine == t_routine)
        return;
    for(auto channel : m_impl->channels)
        delete channel;
    m_impl->channels.clear();
    m_impl->routine = t_routine;

    for(const auto &info : m_impl->routine->channelInfo())
    {
        auto channel = new Channel(info, m_impl->startTime, m_impl->duration, this);
        connect(channel, &Channel::channelUpdated, this, &Clip::channelUpdatedSlot);
        m_impl->channels.append(channel);
    }

    connect(t_routine, &Routine::channelAdded, this, &Clip::channelAddedSlot);
    connect(t_routine, &Routine::channelRemoved, this, &Clip::channelRemovedSlot);
    connect(t_routine, &Routine::channelUpdated, this, &Clip::routineChannelUpdatedSlot);
    emit routineChanged(m_impl->routine);
    m_impl->markChanged();
}

void Clip::channelAddedSlot(int index)
{
    auto channel = new Channel(m_impl->routine->channelInfoAtIndex(index), m_impl->startTime, m_impl->duration, this);
    connect(channel, &Channel::channelUpdated, this, &Clip::channelUpdatedSlot);
    m_impl->channels.append(channel);
    emit channelAdded(channel);
}

void Clip::channelRemovedSlot(int index)
{
    auto channel = m_impl->channels[index];
    m_impl->channels.removeAt(index);
    emit channelRemoved(channel);
    delete channel;
}

void Clip::routineChannelUpdatedSlot(int index)
{
    m_impl->channels[index]->updateInfo(m_impl->routine->channelInfoAtIndex(index));
}


Routine *Clip::routine() const
{
    return m_impl->routine;
}

void Clip::setMask(FixtureMask *t_mask)
{
    if(m_impl->mask == t_mask)
        return;
    m_impl->mask = t_mask;

    //if(m_impl->mask)
      //  connect(m_impl->mask, &FixtureFalloffMap::destroyed, this, [this](){setMask(nullptr);});

    emit maskChanged(m_impl->mask);
    m_impl->markChanged();
}

FixtureMask *Clip::mask() const
{
    return m_impl->mask;
}

void Clip::setStartTime(double t_value)
{
    if(m_impl->startTime == t_value)
        return;
    m_impl->startTime = t_value;
    for(auto channel : m_impl->channels)
    {
        channel->setStartTime(t_value);
    }
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
    for(auto channel : m_impl->channels)
    {
        channel->setDuration(t_value);
    }
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

void Clip::channelUpdatedSlot(Channel *t_channel)
{
    emit channelUpdated(t_channel);
    m_impl->markChanged();
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

void Clip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    for(auto effect : m_impl->falloffEffects)
        delete effect;
    m_impl->falloffEffects.clear();

    m_impl->startTime = t_json.value("startTime").toDouble();
    m_impl->duration = t_json.value("duration").toDouble();
    m_impl->routine = t_context.project->routines()->findByUniqueId(t_json.value("routine").toString().toLatin1());

    auto channelArray = t_json.value("channels").toArray();
    for(auto channelJson : channelArray)
    {
        Channel *channel = new Channel(ChannelInfo(), m_impl->startTime, m_impl->duration, this);
        connect(channel, &Channel::channelUpdated, this, &Clip::channelUpdatedSlot);
        channel->readFromJson(channelJson.toObject(), t_context);
        m_impl->channels.append(channel);
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

}

void Clip::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("startTime", m_impl->startTime);
    t_json.insert("duration", m_impl->duration);
    t_json.insert("routine", QString(m_impl->routine->uniqueId()));

    QJsonArray array;
    for(auto channel : m_impl->channels)
    {
        QJsonObject channelObj;
        channel->writeToJson(channelObj);
        array.append(channelObj);
    }
    t_json.insert("channels", array);

    QJsonArray falloffArray;
    for(auto effect : m_impl->falloffEffects)
    {
        QJsonObject effectObj;
        effectObj.insert("id", QString(effect->id()));
        effect->writeToJson(effectObj);
        falloffArray.append(effectObj);
    }
    t_json.insert("falloffEffects", falloffArray);
}

} // namespace photon
