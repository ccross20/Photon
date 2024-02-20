#include "cliplayer_p.h"
#include "clip_p.h"
#include "routineclip.h"
#include "sequence.h"
#include "plugin/pluginfactory.h"
#include "photoncore.h"

namespace photon {


ClipLayer::Impl::Impl(ClipLayer *t_facade):facade(t_facade)
{

}

void ClipLayer::Impl::addClip(Clip *t_clip)
{
    clips.append(t_clip);
    t_clip->m_impl->setLayer(facade);
    t_clip->setParent(facade);
    t_clip->m_impl->setSequence(facade->sequence());
}

bool ClipLayer::Impl::removeClip(Clip *t_clip)
{
    if(!clips.removeOne(t_clip))
        return false;
    t_clip->m_impl->setLayer(nullptr);
    t_clip->setParent(nullptr);
    t_clip->m_impl->setSequence(nullptr);
    return true;
}

void ClipLayer::Impl::notifyClipWasModified(Clip *t_clip)
{
    emit facade->clipModified(t_clip);
}

ClipLayer::ClipLayer(const QString &t_name, QObject *parent)
    : Layer{t_name, "ClipLayer", parent}, m_impl(new Impl(this))
{
    setName(t_name);
}

ClipLayer::~ClipLayer()
{
    delete m_impl;
}

const QVector<Clip*> &ClipLayer::clips() const
{
    return m_impl->clips;
}

void ClipLayer::addClip(Clip *t_clip)
{
    m_impl->addClip(t_clip);

    emit clipAdded(t_clip);
}

void ClipLayer::removeClip(Clip *t_clip)
{
    if(!m_impl->removeClip(t_clip))
        return;

    emit clipRemoved(t_clip);
}

void ClipLayer::processChannels(ProcessContext &t_context)
{
    for(auto clip : m_impl->clips)
    {
        if(clip->timeIsValid(t_context.globalTime))
        {
            t_context.relativeTime = t_context.globalTime - clip->startTime();
            t_context.channelValues = clip->parameters()->valuesFromChannels(clip->channels(), t_context.relativeTime);

            clip->processChannels(t_context);
        }
    }
}

void ClipLayer::sequenceChanged(Sequence *t_sequence)
{
    Layer::sequenceChanged(t_sequence);
    for(auto clip : m_impl->clips)
        clip->m_impl->setSequence(t_sequence);
}

void ClipLayer::restore(Project &t_project)
{
    Layer::restore(t_project);

    for(auto clip : m_impl->clips)
        clip->restore(t_project);
}

void ClipLayer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Layer::readFromJson(t_json, t_context);

    auto clipArray = t_json.value("clips").toArray();
    for(auto clipJson : clipArray)
    {
        auto clipObj = clipJson.toObject();
        Clip *clip = nullptr;

        qDebug() << "Create clip " << clipObj.value("id");
        clip = photonApp->plugins()->createClip(clipObj.value("id").toString().toLatin1());

        if(clip)
        {
            m_impl->addClip(clip);
            clip->readFromJson(clipObj, t_context);
        }

    }
}

void ClipLayer::writeToJson(QJsonObject &t_json) const
{
    Layer::writeToJson(t_json);

    QJsonArray array;
    for(auto clip : m_impl->clips)
    {
        QJsonObject clipObj;
        clip->writeToJson(clipObj);
        array.append(clipObj);
    }
    t_json.insert("clips", array);
}


} // namespace photon
