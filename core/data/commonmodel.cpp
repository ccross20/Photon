#include "commonmodel.h"
#include "sequence/channel.h"
#include "sequence/channeleffect.h"
#include "falloff/falloffeffect.h"
#include "fixture/maskeffect.h"
#include "sequence/baseeffect.h"

namespace photon {


AbstractTreeData::AbstractTreeData(const QString &name, const QByteArray &id):QObject(),m_name(name),m_id(id)
{

}

AbstractTreeData::~AbstractTreeData()
{

}

QString AbstractTreeData::name()
{
    return m_name;
}

void AbstractTreeData::setName(const QString &t_name)
{
    m_name = t_name;
    emit metadataUpdated(this);
}

AbstractTreeData *AbstractTreeData::findDataWithId(const QByteArray &t_id) const
{
    if(t_id == m_id)
        return const_cast<AbstractTreeData*>(this);

    for(auto child : m_children)
    {
        auto found = child->findDataWithId(t_id);
        if(found)
            return found;
    }

    return nullptr;
}

void AbstractTreeData::insertChild(AbstractTreeData* t_child, int index)
{
    t_child->m_index = index;
    emit childWillBeAdded(this, t_child->index());
    m_children.insert(index, t_child);
    t_child->setParent(this);
    int i = 0;
    for(auto child : m_children)
    {
        child->m_index = i++;
    }

    emit childWasAdded(this);

    connect(t_child, &AbstractTreeData::childWillBeAdded, this, &AbstractTreeData::childWillBeAdded);
    connect(t_child, &AbstractTreeData::childWasAdded, this, &AbstractTreeData::childWasAdded);
    connect(t_child, &AbstractTreeData::childWillBeRemoved, this, &AbstractTreeData::childWillBeRemoved);
    connect(t_child, &AbstractTreeData::childWasRemoved, this, &AbstractTreeData::childWasRemoved);
    connect(t_child, &AbstractTreeData::metadataUpdated, this, &AbstractTreeData::metadataUpdated);
}

void AbstractTreeData::addChild(AbstractTreeData* t_child)
{
    t_child->m_index = m_children.length();
    emit childWillBeAdded(this, t_child->index());
    m_children.append(t_child);
    t_child->setParent(this);
    emit childWasAdded(this);

    connect(t_child, &AbstractTreeData::childWillBeAdded, this, &AbstractTreeData::childWillBeAdded);
    connect(t_child, &AbstractTreeData::childWasAdded, this, &AbstractTreeData::childWasAdded);
    connect(t_child, &AbstractTreeData::childWillBeRemoved, this, &AbstractTreeData::childWillBeRemoved);
    connect(t_child, &AbstractTreeData::childWasRemoved, this, &AbstractTreeData::childWasRemoved);
    connect(t_child, &AbstractTreeData::metadataUpdated, this, &AbstractTreeData::metadataUpdated);
}

void AbstractTreeData::removeChild(AbstractTreeData* t_child)
{
    disconnect(t_child, &AbstractTreeData::childWillBeAdded, this, &AbstractTreeData::childWillBeAdded);
    disconnect(t_child, &AbstractTreeData::childWasAdded, this, &AbstractTreeData::childWasAdded);
    disconnect(t_child, &AbstractTreeData::childWillBeRemoved, this, &AbstractTreeData::childWillBeRemoved);
    disconnect(t_child, &AbstractTreeData::childWasRemoved, this, &AbstractTreeData::childWasRemoved);
    disconnect(t_child, &AbstractTreeData::metadataUpdated, this, &AbstractTreeData::metadataUpdated);

    emit childWillBeRemoved(this, t_child->index());
    m_children.removeAt(t_child->index());

    int i = 0;
    for(auto child : m_children)
    {
        child->m_index = i++;
    }
    emit childWasRemoved(this);
}

RootData::RootData():AbstractTreeData("root","root")
{

}

ChannelData::ChannelData(Channel *t_channel):AbstractTreeData(t_channel->info().name, t_channel->uniqueId()),m_channel(t_channel)
{
    connect(t_channel, &Channel::effectAdded, this, &ChannelData::effectAdded);
    connect(t_channel, &Channel::effectRemoved, this, &ChannelData::effectRemoved);
    connect(t_channel, &Channel::effectMoved, this, &ChannelData::effectMoved);
    connect(t_channel, &Channel::channelUpdated, this, &ChannelData::channelUpdated);


    if(t_channel->subChannelCount() > 0)
    {
        m_subChannelFolder = new FolderData("Sub-Channels", DataChannel, false);
        addChild(m_subChannelFolder);
        for(int i = 0; i < t_channel->subChannelCount(); ++i)
        {
            auto channelData = new ChannelData(t_channel->subChannels()[i]);
            m_subChannelFolder->addChild(channelData);
        }
    }


    for(int i = 0; i < t_channel->effectCount(); ++i)
    {
        auto effectData = new ChannelEffectData(t_channel->effectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

ChannelEffectData *ChannelData::findEffectData(ChannelEffect *t_effect)
{
    for(auto child : children())
    {
        ChannelEffectData *childData = dynamic_cast<ChannelEffectData*>(child);
        if(childData && childData->effect() == t_effect)
        {
            return childData;
        }
    }
    return nullptr;
}

void ChannelData::channelUpdated()
{
    setName(m_channel->info().name);
}

void ChannelData::effectAdded(photon::ChannelEffect *t_effect)
{
    auto effectData = new ChannelEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ChannelData::effectRemoved(photon::ChannelEffect *t_effect)
{
    auto effectData = findEffectData(t_effect);

    if(effectData)
        removeChild(effectData);
}

void ChannelData::effectMoved(photon::ChannelEffect *t_effect)
{

}

FolderData::FolderData(const QString &name, TreeDataType allowedTypes, bool showCreateItem): AbstractTreeData(name),m_showCreate(showCreateItem),m_allowedTypes(allowedTypes)
{
    if(showCreateItem)
    {
        addChild(new CreateData("Add..."));
    }
}

ChannelEffectData::ChannelEffectData(ChannelEffect *t_effect):AbstractTreeData(t_effect->name(), t_effect->uniqueId()),m_effect(t_effect)
{

}



FalloffEffectData::FalloffEffectData(FalloffEffect *t_effect):AbstractTreeData(t_effect->name(), t_effect->uniqueId()),m_effect(t_effect)
{

}


BaseEffectData::BaseEffectData(BaseEffect *t_effect):AbstractTreeData(t_effect->name(), t_effect->uniqueId()),m_effect(t_effect)
{
    connect(t_effect, &BaseEffect::channelAdded, this, &BaseEffectData::channelAdded);
    connect(t_effect, &BaseEffect::channelRemoved, this, &BaseEffectData::channelRemoved);

    for(int i = 0; i < t_effect->channelCount(); ++i)
    {
        auto channelData = new ChannelData(t_effect->channelAtIndex(i));
        addChild(channelData);
    }
}

void BaseEffectData::channelAdded(photon::Channel *t_channel)
{
    qDebug() << "Added";
    auto channelData = new ChannelData(t_channel);
    addChild(channelData);
}

void BaseEffectData::channelRemoved(photon::Channel *t_channel)
{
    for(auto child : children())
    {
        auto channelChild = dynamic_cast<ChannelData*>(child);
        if(channelChild && channelChild->channel() == t_channel)
        {
            removeChild(child);
            return;
        }
    }
}

void BaseEffectData::channelMoved(photon::Channel *)
{

}


CreateData::CreateData(const QString &name): AbstractTreeData(name)
{

}



} // namespace photon
