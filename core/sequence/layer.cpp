#include "layer_p.h"
#include "clip_p.h"
#include "sequence.h"

namespace photon {

Layer::Impl::Impl(Layer *t_facade):facade(t_facade)
{

}

void Layer::Impl::addClip(Clip *t_clip)
{
    clips.append(t_clip);
    t_clip->m_impl->setLayer(facade);
    t_clip->setParent(facade);
    t_clip->m_impl->setSequence(sequence);
}

void Layer::Impl::removeClip(Clip *t_clip)
{
    clips.removeOne(t_clip);
    t_clip->m_impl->setLayer(nullptr);
    t_clip->setParent(nullptr);
    t_clip->m_impl->setSequence(nullptr);
}

void Layer::Impl::setSequence(Sequence *t_sequence)
{
    sequence = t_sequence;
    for(auto clip : clips)
        clip->m_impl->setSequence(t_sequence);
}

void Layer::Impl::notifyClipWasModified(Clip *t_clip)
{
    emit facade->clipModified(t_clip);
}

Layer::Layer(const QString &t_name, QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{
    setName(t_name);
}

Layer::~Layer()
{
    delete m_impl;
}

QString Layer::name() const
{
    return m_impl->name;
}

void Layer::setName(const QString &name)
{
    m_impl->name = name;
    setObjectName(name);
}

Sequence *Layer::sequence() const
{
    return m_impl->sequence;
}

const QVector<Clip*> &Layer::clips() const
{
    return m_impl->clips;
}

void Layer::addClip(Clip *t_clip)
{
    m_impl->addClip(t_clip);

    emit clipAdded(t_clip);
}

void Layer::removeClip(Clip *t_clip)
{
    m_impl->removeClip(t_clip);

    emit clipRemoved(t_clip);
}

int Layer::height() const
{
    return 50;
}

void Layer::processChannels(ProcessContext &t_context)
{
    for(auto clip : m_impl->clips)
    {
        if(clip->startTime() > t_context.globalTime)
            return;
        if(clip->endTime() > t_context.globalTime)
        {
            clip->processChannels(t_context);
            return;
        }
    }
}

void Layer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();

    auto clipArray = t_json.value("clips").toArray();
    for(auto clipJson : clipArray)
    {
        auto clip = new Clip;
        m_impl->addClip(clip);
        clip->readFromJson(clipJson.toObject(), t_context);
    }
}

void Layer::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
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
