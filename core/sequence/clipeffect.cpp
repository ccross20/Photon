#include <QColor>
#include "clipeffect_p.h"
#include "clip.h"
#include "sequence.h"

namespace photon {

ClipEffectEvaluationContext ClipEffect::Impl::processFixture(Fixture *t_fixture, ClipEffectEvaluationContext &t_context, double initialRelativeTime)
{
    t_context.relativeTime = initialRelativeTime - clip->falloff(t_fixture);
    if(t_context.relativeTime < 0)
        return t_context;

    t_context.fixture = t_fixture;
    t_context.strength = clip->strengthAtTime(t_context.relativeTime);

    t_context.channelValues.clear();
    for(const auto &channel : channels)
    {
        t_context.channelValues.insert(channel->uniqueId(), channel->processDouble(t_context.relativeTime));
    }
    return t_context;
}

ClipEffect::ClipEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

ClipEffect::~ClipEffect()
{
    delete m_impl;
}

void ClipEffect::processChannels(ProcessContext &t_context) const
{
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    ClipEffectEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.fixture = t_context.fixture;
    localContext.canvasImage = t_context.canvasImage;
    localContext.previousCanvasImage = t_context.previousCanvasImage;
    localContext.project = t_context.project;
    localContext.fixtureTotal = clip()->maskedFixtures().length();
    int index = 0;
    for(auto fixture : clip()->maskedFixtures())
    {
        localContext.fixtureIndex = index++;
        auto t = m_impl->processFixture(fixture, localContext, initialRelativeTime);
        t.fixture = fixture;
        if(fixture)
            evaluateFixture(t);
    }

}

void ClipEffect::evaluateFixture(ClipEffectEvaluationContext &) const
{

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

QWidget *ClipEffect::createEditor()
{
    return nullptr;
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

Channel *ClipEffect::channelAtIndex(int t_index) const
{
    return m_impl->channels[t_index];
}

int ClipEffect::channelCount() const
{
    return m_impl->channels.length();
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
}


} // namespace photon
