#include "actionmodel.h"
#include "surface/surfaceaction.h"
#include "sequence/masterlayer.h"
#include "sequence/channel.h"
#include "sequence/channeleffect.h"
#include "falloff/falloffeffect.h"
#include "fixture/maskeffect.h"
#include "surface/fixtureaction.h"
#include "state/state.h"
#include "sequence/baseeffect.h"
#include "pixel/pixellayout.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/canvasaction.h"

namespace photon {



ActionData::ActionData(SurfaceAction *t_clip) : AbstractTreeData(t_clip->name(), t_clip->uniqueId()),m_action(t_clip)
{

    m_parameterData = new ActionParameterData(m_action);
    m_actionEffectData = new ActionEffectFolderData(m_action);

    addChild(m_parameterData);

    auto fixtureClip = dynamic_cast<FixtureAction*>(t_clip);
    auto canvasClip = dynamic_cast<CanvasAction*>(t_clip);


    if(fixtureClip)
    {
        addChild(new ActionStateData(fixtureClip));
        m_maskFolder = new ActionMaskData(fixtureClip);
        m_falloffData = new ActionFalloffData(fixtureClip);
    }

    if(fixtureClip || canvasClip)
        addChild(m_actionEffectData);

    if(fixtureClip)
    {
        addChild(m_maskFolder);
        addChild(m_falloffData);
    }

    if(t_clip)
    {
        m_channelFolder = new FolderData("Channels", DataChannel, false);
        connect(m_action, &SurfaceAction::channelAdded, this, &ActionData::channelAdded);
        connect(m_action, &SurfaceAction::channelRemoved, this, &ActionData::channelRemoved);
        addChild(m_channelFolder);
        for(int i = 0; i < m_action->channelCount(); ++i)
        {
            auto channelData = new ChannelData(m_action->channelAtIndex(i));
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

void ActionData::channelAdded(photon::Channel *t_channel)
{
    qDebug() << "Added";
    auto channelData = new ChannelData(t_channel);
    m_channelFolder->addChild(channelData);
}

void ActionData::channelRemoved(photon::Channel *t_channel)
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

void ActionData::channelMoved(photon::Channel *)
{

}




ActionParameterData::ActionParameterData(SurfaceAction *t_clip): AbstractTreeData("Parameters","parameters"),m_action(t_clip)
{

}

ActionEffectFolderData::ActionEffectFolderData(SurfaceAction *t_clip): AbstractTreeData("Effects","effects"),m_action(t_clip)
{
    connect(m_action, &SurfaceAction::clipEffectAdded, this, &ActionEffectFolderData::effectAdded);
    connect(m_action, &SurfaceAction::clipEffectRemoved, this, &ActionEffectFolderData::effectRemoved);

    qDebug() << "Effect count" << t_clip->clipEffectCount();
    for(int i = 0; i < t_clip->clipEffectCount(); ++i)
    {
        auto effectData = new BaseEffectData(m_action->clipEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

BaseEffectData *ActionEffectFolderData::findEffectData(BaseEffect *t_effect)
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

void ActionEffectFolderData::effectAdded(photon::BaseEffect *t_effect)
{
    auto effectData = new BaseEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ActionEffectFolderData::effectRemoved(photon::BaseEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ActionEffectFolderData::effectMoved(photon::BaseEffect *t_effect)
{

}

ActionStateData::ActionStateData(FixtureAction *t_clip):AbstractTreeData("State", t_clip->state()->uniqueId()),m_action(t_clip)
{

}

State *ActionStateData::state() const
{
    return m_action->state();
}

ActionFalloffData::ActionFalloffData(FixtureAction *t_clip): AbstractTreeData("Falloff","falloff"),m_action(t_clip)
{
    connect(m_action, &FixtureAction::falloffEffectAdded, this, &ActionFalloffData::effectAdded);
    connect(m_action, &FixtureAction::falloffEffectRemoved, this, &ActionFalloffData::effectRemoved);

    for(int i = 0; i < t_clip->falloffEffectCount(); ++i)
    {
        auto effectData = new FalloffEffectData(m_action->falloffEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

FalloffEffectData *ActionFalloffData::findEffectData(FalloffEffect *t_effect)
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

void ActionFalloffData::effectAdded(photon::FalloffEffect *t_effect)
{
    auto effectData = new FalloffEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ActionFalloffData::effectRemoved(photon::FalloffEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ActionFalloffData::effectMoved(photon::FalloffEffect *t_effect)
{

}

ActionMaskData::ActionMaskData(FixtureAction *t_clip): AbstractTreeData("Selection Mask","mask"),m_action(t_clip)
{
    connect(m_action, &FixtureAction::maskAdded, this, &ActionMaskData::effectAdded);
    connect(m_action, &FixtureAction::maskRemoved, this, &ActionMaskData::effectRemoved);

    for(int i = 0; i < t_clip->maskEffectCount(); ++i)
    {
        auto effectData = new MaskEffectData(m_action->maskEffectAtIndex(i));
        addChild(effectData);
    }
    addChild(new CreateData("Add Effect..."));
}

MaskEffectData *ActionMaskData::findEffectData(MaskEffect *t_effect)
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

void ActionMaskData::effectAdded(photon::MaskEffect *t_effect)
{
    auto effectData = new MaskEffectData(t_effect);
    insertChild(effectData, childCount() - 1);
}

void ActionMaskData::effectRemoved(photon::MaskEffect *t_effect)
{
    auto effect = findEffectData(t_effect);

    if(effect)
        removeChild(effect);
}

void ActionMaskData::effectMoved(photon::MaskEffect *t_effect)
{

}

MaskEffectData::MaskEffectData(MaskEffect *t_effect):AbstractTreeData(t_effect->name(), t_effect->uniqueId()), m_effect(t_effect)
{

}

ActionModel::ActionModel()
{
    m_root = new RootData();
    connect(m_root, &AbstractTreeData::childWillBeAdded, this, &ActionModel::childWillBeAdded);
    connect(m_root, &AbstractTreeData::childWasAdded, this, &ActionModel::childWasAdded);
    connect(m_root, &AbstractTreeData::childWillBeRemoved, this, &ActionModel::childWillBeRemoved);
    connect(m_root, &AbstractTreeData::childWasRemoved, this, &ActionModel::childWasRemoved);
    connect(m_root, &AbstractTreeData::metadataUpdated, this, &ActionModel::metadataUpdated);
}

ActionModel::~ActionModel()
{
    delete m_root;
}

void ActionModel::addAction(SurfaceAction *t_clip)
{
    ActionData *actionData = new ActionData(t_clip);
    m_root->addChild(actionData);
}

void ActionModel::removeAction(SurfaceAction *t_clip)
{
    for(int i = 0; i < m_root->childCount(); ++i)
    {
        ActionData *actionData = dynamic_cast<ActionData*>(m_root->childAtIndex(i));
        if(actionData->action() == t_clip)
        {
            m_root->removeChild(actionData);
            return;
        }
    }
}

QVector<SurfaceAction*> ActionModel::actions() const
{
    QVector<SurfaceAction*> results;
    for(int i = 0; i < m_root->childCount(); ++i)
    {
        ActionData *actionData = dynamic_cast<ActionData*>(m_root->childAtIndex(i));
        if(actionData)
        {
            results.append(actionData->action());
        }
    }
    return results;
}

void ActionModel::childWillBeAdded(photon::AbstractTreeData* t_data, int t_index)
{
    QModelIndex parentIndex = indexForData(t_data);

    if(t_index >= 0)
        beginInsertRows(parentIndex,t_index,t_index);
}

void ActionModel::childWasAdded(photon::AbstractTreeData*)
{
    endInsertRows();
}

void ActionModel::metadataUpdated(photon::AbstractTreeData* t_data)
{
    auto index = indexForData(static_cast<AbstractTreeData*>(t_data));
    emit dataChanged(index, index);
}

void ActionModel::childWillBeRemoved(photon::AbstractTreeData* t_data, int t_index)
{
    QModelIndex parentIndex = indexForData(t_data);

    if(t_index >= 0)
        beginRemoveRows(parentIndex,t_index,t_index);
}

void ActionModel::childWasRemoved(photon::AbstractTreeData*)
{
    endRemoveRows();
}

void ActionModel::setContainer(SurfaceGizmoContainer *t_container)
{
    m_container = t_container;

    for(auto action : m_container->actions())
        addAction(action);
    connect(m_container, &SurfaceGizmoContainer::actionWasAdded,this, &ActionModel::actionWasAdded);
    connect(m_container, &SurfaceGizmoContainer::actionWasRemoved,this, &ActionModel::actionWasRemoved);
}

void ActionModel::actionWasAdded(photon::SurfaceAction *t_action)
{
    addAction(t_action);
}

void ActionModel::actionWasRemoved(photon::SurfaceAction *t_action)
{
    removeAction(t_action);
}

QModelIndex ActionModel::indexForData(AbstractTreeData *t_data) const
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

QModelIndex ActionModel::indexForId(const QByteArray &t_id) const
{
    auto data = m_root->findDataWithId(t_id);
    if(data){
        return indexForData(data);
    }
    return QModelIndex{};
}

AbstractTreeData *ActionModel::dataForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    else
        return static_cast<AbstractTreeData*>(index.internalPointer());
}

Qt::ItemFlags ActionModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    if(index.column() == 0)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

//! [2]
int ActionModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}
//! [2]

QVariant ActionModel::data(const QModelIndex &index, int role) const
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

QVariant ActionModel::headerData(int section, Qt::Orientation orientation,
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
QModelIndex ActionModel::index(int row, int column, const QModelIndex &parent) const
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
QModelIndex ActionModel::parent(const QModelIndex &index) const
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
int ActionModel::rowCount(const QModelIndex &parent) const
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

bool ActionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;


    AbstractTreeData *treeData = dataForIndex(index);
    if(treeData)
    {
        if(index.column() == 0 && !dataForIndex(index.parent()))
        {
            treeData->setName(value.toString());
            auto action = dynamic_cast<ActionData*>(treeData);
            if(action)
                action->action()->setName(value.toString());
        }

    }



    if(index.column() == 0)
    {
        //layerForIndex(index)->setName(value.toString());
        emit dataChanged(index,index);
        return true;
    }


    return false;
}

} // namespace photon
