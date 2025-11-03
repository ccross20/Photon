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
#include "sequence/baseeffect.h"
#include "sequence/canvaslayergroup.h"
#include "pixel/pixellayout.h"

namespace photon {


MasterLayerData::MasterLayerData(MasterLayer *t_layer) : AbstractTreeData(t_layer->name(), t_layer->uniqueId()),m_layer(t_layer)
{
    m_channelFolder = new FolderData("Channels", DataChannel, false);
    addChild(m_channelFolder);

    auto channelData = new ChannelData(t_layer->channel());
    m_channelFolder->addChild(channelData);

}



ClipData::ClipData(Clip *t_clip) : AbstractTreeData(t_clip->name(), t_clip->uniqueId()),m_clip(t_clip)
{

    m_parameterData = new ClipParameterData(m_clip);
    m_clipEffectData = new ClipEffectFolderData(m_clip);

    addChild(m_parameterData);

    auto fixtureClip = dynamic_cast<FixtureClip*>(t_clip);
    auto canvasClip = dynamic_cast<CanvasClip*>(t_clip);


    if(fixtureClip)
    {
        addChild(new ClipStateData(fixtureClip));
        m_maskFolder = new ClipMaskData(fixtureClip);
        m_falloffData = new ClipFalloffData(fixtureClip);
    }

    if(fixtureClip || canvasClip)
        addChild(m_clipEffectData);

    if(fixtureClip)
    {
        addChild(m_maskFolder);
        addChild(m_falloffData);
    }

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

    /*
    if(canvasClip)
    {
        m_pixelLayoutData = new PixelLayoutFolderData(canvasClip);
        addChild(m_pixelLayoutData);
    }
*/


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




ClipParameterData::ClipParameterData(Clip *t_clip): AbstractTreeData("Parameters","parameters"),m_clip(t_clip)
{

}

ClipEffectFolderData::ClipEffectFolderData(Clip *t_clip): AbstractTreeData("Effects","effects"),m_clip(t_clip)
{
    connect(m_clip, &Clip::clipEffectAdded, this, &ClipEffectFolderData::effectAdded);
    connect(m_clip, &Clip::clipEffectRemoved, this, &ClipEffectFolderData::effectRemoved);

    qDebug() << "Effect count" << t_clip->clipEffectCount();
    for(int i = 0; i < t_clip->clipEffectCount(); ++i)
    {
        auto effectData = new BaseEffectData(m_clip->clipEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

BaseEffectData *ClipEffectFolderData::findEffectData(BaseEffect *t_effect)
{
    for(auto child : children())
    {
        BaseEffectData *childData = dynamic_cast<BaseEffectData*>(child);
        if(childData && childData->effect() == t_effect)
        {
            return childData;
        }
    }
    return nullptr;
}

void ClipEffectFolderData::effectAdded(photon::BaseEffect *t_effect)
{
    auto effectData = new BaseEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ClipEffectFolderData::effectRemoved(photon::BaseEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ClipEffectFolderData::effectMoved(photon::BaseEffect *t_effect)
{

}

PixelLayoutData::PixelLayoutData(PixelLayout *t_layout):AbstractTreeData(t_layout->name(), t_layout->uniqueId())
{

}

PixelLayoutFolderData::PixelLayoutFolderData(CanvasLayerGroup *t_clip):AbstractTreeData("Pixel Layout", "pixel-layout"),m_layer(t_clip)
{
    connect(m_layer, &CanvasLayerGroup::pixelLayoutAdded, this, &PixelLayoutFolderData::pixelLayoutAdded);
    connect(m_layer, &CanvasLayerGroup::pixelLayoutRemoved, this, &PixelLayoutFolderData::pixelLayoutRemoved);

    for(int i = 0; i < t_clip->pixelLayoutCount(); ++i)
    {
        auto effectData = new PixelLayoutData(m_layer->pixelLayoutAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Pixel Layout..."));
}

PixelLayoutData *PixelLayoutFolderData::findPixelLayoutData(PixelLayout *t_layout)
{
    for(auto child : children())
    {
        PixelLayoutData *childData = dynamic_cast<PixelLayoutData*>(child);
        if(childData && childData->pixelLayout() == t_layout)
        {
            return childData;
        }
    }
    return nullptr;
}

void PixelLayoutFolderData::pixelLayoutAdded(photon::PixelLayout *t_layout)
{
    auto effectData = new PixelLayoutData(t_layout);
    insertChild(effectData, childCount() - 1);
}

void PixelLayoutFolderData::pixelLayoutRemoved(photon::PixelLayout *t_layout)
{
    auto effect = findPixelLayoutData(t_layout);

    if(effect)
        removeChild(effect);
}

ClipStateData::ClipStateData(FixtureClip *t_clip):AbstractTreeData("State", t_clip->state()->uniqueId()),m_clip(t_clip)
{

}

State *ClipStateData::state() const
{
    return m_clip->state();
}

ClipFalloffData::ClipFalloffData(FixtureClip *t_clip): AbstractTreeData("Falloff","falloff"),m_clip(t_clip)
{
    connect(m_clip, &FixtureClip::falloffEffectAdded, this, &ClipFalloffData::effectAdded);
    connect(m_clip, &FixtureClip::falloffEffectRemoved, this, &ClipFalloffData::effectRemoved);

    for(int i = 0; i < t_clip->falloffEffectCount(); ++i)
    {
        auto effectData = new FalloffEffectData(m_clip->falloffEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

FalloffEffectData *ClipFalloffData::findEffectData(FalloffEffect *t_effect)
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

void ClipFalloffData::effectAdded(photon::FalloffEffect *t_effect)
{
    auto effectData = new FalloffEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ClipFalloffData::effectRemoved(photon::FalloffEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ClipFalloffData::effectMoved(photon::FalloffEffect *t_effect)
{

}

ClipMaskData::ClipMaskData(FixtureClip *t_clip): AbstractTreeData("Selection Mask","mask"),m_clip(t_clip)
{
    connect(m_clip, &FixtureClip::maskAdded, this, &ClipMaskData::effectAdded);
    connect(m_clip, &FixtureClip::maskRemoved, this, &ClipMaskData::effectRemoved);

    for(int i = 0; i < t_clip->maskEffectCount(); ++i)
    {
        auto effectData = new MaskEffectData(m_clip->maskEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

MaskEffectData *ClipMaskData::findEffectData(MaskEffect *t_effect)
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

void ClipMaskData::effectAdded(photon::MaskEffect *t_effect)
{
    auto effectData = new MaskEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ClipMaskData::effectRemoved(photon::MaskEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ClipMaskData::effectMoved(photon::MaskEffect *t_effect)
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
