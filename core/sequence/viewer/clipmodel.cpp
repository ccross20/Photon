#include "clipmodel.h"
#include "sequence/clip.h"
#include "sequence/masterlayer.h"
#include "sequence/channel.h"
#include "sequence/channeleffect.h"
#include "falloff/falloffeffect.h"
#include "fixture/maskeffect.h"
#include "sequence/routineclip.h"
#include "sequence/fixtureclip.h"
#include "sequence/canvasclip.h"
#include "sequence/clipeffect.h"

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


MasterLayerData::MasterLayerData(MasterLayer *t_layer) : AbstractTreeData(t_layer->name(), t_layer->uniqueId()),m_layer(t_layer)
{
    m_channelFolder = new FolderData("Channels", DataChannel, false);
    addChild(m_channelFolder);

    auto channelData = new ChannelData(t_layer->channel());
    m_channelFolder->addChild(channelData);

}



ClipData::ClipData(Clip *t_clip) : AbstractTreeData(t_clip->name(), t_clip->uniqueId()),m_clip(t_clip)
{
    m_maskFolder = new MaskData(m_clip);
    m_falloffData = new FalloffData(m_clip);
    m_clipEffectData = new ClipEffectFolderData(m_clip);


    auto fixtureClip = dynamic_cast<FixtureClip*>(t_clip);
    auto canvasClip = dynamic_cast<CanvasClip*>(t_clip);


    if(fixtureClip)
    {
        addChild(new StateData(fixtureClip));
    }

    if(fixtureClip || canvasClip)
        addChild(m_clipEffectData);


    addChild(m_maskFolder);
    addChild(m_falloffData);

    if(t_clip)
    {
        m_channelFolder = new FolderData("Channels", DataChannel, false);
        connect(t_clip, &Clip::channelAdded, this, &ClipData::channelAdded);
        connect(t_clip, &Clip::channelRemoved, this, &ClipData::channelRemoved);
        addChild(m_channelFolder);
        for(int i = 0; i < t_clip->channelCount(); ++i)
        {
            auto channelData = new ChannelData(t_clip->channelAtIndex(i));
            m_channelFolder->addChild(channelData);
        }
    }

}

void ClipData::channelAdded(photon::Channel *t_channel)
{
    qDebug() << "Added";
    auto channelData = new ChannelData(t_channel);
    m_channelFolder->addChild(channelData);
}

void ClipData::channelRemoved(photon::Channel *t_channel)
{
    for(auto child : m_channelFolder->children())
    {
        auto channelChild = dynamic_cast<ChannelData*>(child);
        if(channelChild && channelChild->channel() == t_channel)
        {
            m_channelFolder->removeChild(child);
            return;
        }
    }
}

void ClipData::channelMoved(photon::Channel *)
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


ClipEffectFolderData::ClipEffectFolderData(Clip *t_clip): AbstractTreeData("Effects","effects"),m_clip(t_clip)
{
    connect(m_clip, &Clip::clipEffectAdded, this, &ClipEffectFolderData::effectAdded);
    connect(m_clip, &Clip::clipEffectRemoved, this, &ClipEffectFolderData::effectRemoved);

    qDebug() << "Effect count" << t_clip->clipEffectCount();
    for(int i = 0; i < t_clip->clipEffectCount(); ++i)
    {
        auto effectData = new ClipEffectData(m_clip->clipEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

ClipEffectData *ClipEffectFolderData::findEffectData(ClipEffect *t_effect)
{
    for(auto child : children())
    {
        ClipEffectData *childData = dynamic_cast<ClipEffectData*>(child);
        if(childData && childData->effect() == t_effect)
        {
            return childData;
        }
    }
    return nullptr;
}

void ClipEffectFolderData::effectAdded(photon::ClipEffect *t_effect)
{
    auto effectData = new ClipEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ClipEffectFolderData::effectRemoved(photon::ClipEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ClipEffectFolderData::effectMoved(photon::ClipEffect *t_effect)
{

}

ClipEffectData::ClipEffectData(ClipEffect *t_effect):AbstractTreeData(t_effect->name(), t_effect->uniqueId()),m_effect(t_effect)
{
    connect(t_effect, &ClipEffect::channelAdded, this, &ClipEffectData::channelAdded);
    connect(t_effect, &ClipEffect::channelRemoved, this, &ClipEffectData::channelRemoved);

    for(int i = 0; i < t_effect->channelCount(); ++i)
    {
        auto channelData = new ChannelData(t_effect->channelAtIndex(i));
        addChild(channelData);
    }
}

void ClipEffectData::channelAdded(photon::Channel *t_channel)
{
    qDebug() << "Added";
    auto channelData = new ChannelData(t_channel);
    addChild(channelData);
}

void ClipEffectData::channelRemoved(photon::Channel *t_channel)
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

void ClipEffectData::channelMoved(photon::Channel *)
{

}


StateData::StateData(FixtureClip *t_clip):AbstractTreeData("State", t_clip->state()->uniqueId()),m_clip(t_clip)
{

}

State *StateData::state() const
{
    return m_clip->state();
}

CreateData::CreateData(const QString &name): AbstractTreeData(name)
{

}

FalloffData::FalloffData(Clip *t_clip): AbstractTreeData("Falloff","falloff"),m_clip(t_clip)
{
    connect(m_clip, &Clip::falloffEffectAdded, this, &FalloffData::effectAdded);
    connect(m_clip, &Clip::falloffEffectRemoved, this, &FalloffData::effectRemoved);

    for(int i = 0; i < t_clip->falloffEffectCount(); ++i)
    {
        auto effectData = new FalloffEffectData(m_clip->falloffEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

FalloffEffectData *FalloffData::findEffectData(FalloffEffect *t_effect)
{
    for(auto child : children())
    {
        FalloffEffectData *childData = dynamic_cast<FalloffEffectData*>(child);
        if(childData && childData->effect() == t_effect)
        {
            return childData;
        }
    }
    return nullptr;
}

void FalloffData::effectAdded(photon::FalloffEffect *t_effect)
{
    auto effectData = new FalloffEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void FalloffData::effectRemoved(photon::FalloffEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void FalloffData::effectMoved(photon::FalloffEffect *t_effect)
{

}

MaskData::MaskData(Clip *t_clip): AbstractTreeData("Selection Mask","mask"),m_clip(t_clip)
{
    connect(m_clip, &Clip::maskAdded, this, &MaskData::effectAdded);
    connect(m_clip, &Clip::maskRemoved, this, &MaskData::effectRemoved);

    for(int i = 0; i < t_clip->maskEffectCount(); ++i)
    {
        auto effectData = new MaskEffectData(m_clip->maskEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

MaskEffectData *MaskData::findEffectData(MaskEffect *t_effect)
{
    for(auto child : children())
    {
        MaskEffectData *childData = dynamic_cast<MaskEffectData*>(child);
        if(childData && childData->effect() == t_effect)
        {
            return childData;
        }
    }
    return nullptr;
}

void MaskData::effectAdded(photon::MaskEffect *t_effect)
{
    auto effectData = new MaskEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void MaskData::effectRemoved(photon::MaskEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void MaskData::effectMoved(photon::MaskEffect *t_effect)
{

}

MaskEffectData::MaskEffectData(MaskEffect *t_effect):AbstractTreeData(t_effect->name(), t_effect->uniqueId()), m_effect(t_effect)
{

}

ClipModel::ClipModel()
{
    m_root = new RootData();
    connect(m_root, &AbstractTreeData::childWillBeAdded, this, &ClipModel::childWillBeAdded);
    connect(m_root, &AbstractTreeData::childWasAdded, this, &ClipModel::childWasAdded);
    connect(m_root, &AbstractTreeData::childWillBeRemoved, this, &ClipModel::childWillBeRemoved);
    connect(m_root, &AbstractTreeData::childWasRemoved, this, &ClipModel::childWasRemoved);
    connect(m_root, &AbstractTreeData::metadataUpdated, this, &ClipModel::metadataUpdated);
}

ClipModel::~ClipModel()
{
    delete m_root;
}

void ClipModel::addClip(Clip *t_clip)
{
    ClipData *clipData = new ClipData(t_clip);
    m_root->addChild(clipData);
}

void ClipModel::removeClip(Clip *t_clip)
{
    for(int i = 0; i < m_root->childCount(); ++i)
    {
        ClipData *clipData = dynamic_cast<ClipData*>(m_root->childAtIndex(i));
        if(clipData->clip() == t_clip)
        {
            m_root->removeChild(clipData);
            return;
        }
    }
}

QVector<Clip*> ClipModel::clips() const
{
    QVector<Clip*> results;
    for(int i = 0; i < m_root->childCount(); ++i)
    {
        ClipData *clipData = dynamic_cast<ClipData*>(m_root->childAtIndex(i));
        if(clipData)
        {
            results.append(clipData->clip());
        }
    }
    return results;
}

void ClipModel::addMasterLayer(MasterLayer *t_layer)
{
    m_root->addChild(new MasterLayerData(t_layer));
}

void ClipModel::removeMasterLayer(MasterLayer *t_layer)
{
    for(int i = 0; i < m_root->childCount(); ++i)
    {
        MasterLayerData *layerData = dynamic_cast<MasterLayerData*>(m_root->childAtIndex(i));
        if(layerData->layer() == t_layer)
        {
            m_root->removeChild(layerData);
            return;
        }
    }
}

void ClipModel::childWillBeAdded(photon::AbstractTreeData* t_data, int t_index)
{
    QModelIndex parentIndex = indexForData(t_data);

    if(t_index >= 0)
        beginInsertRows(parentIndex,t_index,t_index);
}

void ClipModel::childWasAdded(photon::AbstractTreeData*)
{
    endInsertRows();
}

void ClipModel::metadataUpdated(photon::AbstractTreeData* t_data)
{
    auto index = indexForData(static_cast<AbstractTreeData*>(t_data));
    emit dataChanged(index, index);
}

void ClipModel::childWillBeRemoved(photon::AbstractTreeData* t_data, int t_index)
{
    QModelIndex parentIndex = indexForData(t_data);

    if(t_index >= 0)
        beginRemoveRows(parentIndex,t_index,t_index);
}

void ClipModel::childWasRemoved(photon::AbstractTreeData*)
{
    endRemoveRows();
}

QModelIndex ClipModel::indexForData(AbstractTreeData *t_data) const
{
    if(t_data == nullptr)
        return QModelIndex();
    if(t_data->parent())
    {
        return index(t_data->index(),0, indexForData(static_cast<AbstractTreeData*>(t_data->parent())));
    } else {
        return index(t_data->index(),0);
    }
}

QModelIndex ClipModel::indexForId(const QByteArray &t_id) const
{
    auto data = m_root->findDataWithId(t_id);
    if(data){
        return indexForData(data);
    }
    return QModelIndex{};
}

AbstractTreeData *ClipModel::dataForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    else
        return static_cast<AbstractTreeData*>(index.internalPointer());
}

Qt::ItemFlags ClipModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    if(index.column() == 0)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

//! [2]
int ClipModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}
//! [2]

QVariant ClipModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            AbstractTreeData *treeData = dataForIndex(index);
            if(treeData)
            {
                if(index.column() == 0)
                    return treeData->name();

            }
        }
            break;

    }

    return QVariant();
}

QVariant ClipModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(section == 0)
            return "Name";

        return "";
    }

/*
    if(role == Qt::SizeHintRole)
    {
        if(section > 0 && section <= m_channelCount )
            return QSize{32,32};
    }
    */

    return QVariant();
}

//! [5]
QModelIndex ClipModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractTreeData *parentData = dataForIndex(parent);
    if(!parentData)
        return createIndex(row, column, m_root->childAtIndex(row));
    AbstractTreeData *childData = parentData->childAtIndex(row);
    if (childData)
        return createIndex(row, column, childData);
    else
        return QModelIndex();

}
//! [6]

//! [7]
QModelIndex ClipModel::parent(const QModelIndex &index) const
{
    AbstractTreeData *childData = dataForIndex(index);
    if (!childData)
        return QModelIndex();

    AbstractTreeData *parentData = static_cast<AbstractTreeData*>(childData->parent());
    if (!parentData)
        return QModelIndex();

    return createIndex(parentData->index(), 0, parentData);
}
//! [7]

//! [8]
int ClipModel::rowCount(const QModelIndex &parent) const
{
    if(!m_root)return 0;

    if (parent.column() > 0)
        return 0;


    AbstractTreeData *parentData = dataForIndex(parent);
    if(parentData)
        return parentData->childCount();
    else
    {
        return m_root->childCount();
    }
}
//! [8]

bool ClipModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if(index.column() == 0)
    {
        //layerForIndex(index)->setName(value.toString());
        emit dataChanged(index,index);
        return true;
    }

    return false;
}

} // namespace photon
