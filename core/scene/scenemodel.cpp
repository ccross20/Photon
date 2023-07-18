#include <QMimeData>
#include <QIODevice>
#include <QSize>
#include <QCoreApplication>
#include "scenemodel.h"
#include "sceneobject.h"
#include "fixture/fixture.h"

namespace photon {

class SceneModel::Impl
{
public:
    SceneObject *rootObject;

};

SceneModel::SceneModel(SceneObject *root, QObject *parent)
    : QAbstractItemModel{parent},m_impl(new Impl)
{
    m_impl->rootObject = root;

    connect(root, &SceneObject::childWillBeAdded, this, &SceneModel::childWillBeAdded);
    connect(root, &SceneObject::childWasAdded, this, &SceneModel::childWasAdded);
    connect(root, &SceneObject::childWillBeRemoved, this, &SceneModel::childWillBeRemoved);
    connect(root, &SceneObject::childWasRemoved, this, &SceneModel::childWasRemoved);
    connect(root, &SceneObject::childWillBeMoved, this, &SceneModel::childWillBeMoved);
    connect(root, &SceneObject::childWasMoved, this, &SceneModel::childWasMoved);
    connect(root, &SceneObject::descendantModified, this, &SceneModel::objectWasUpdated);
}

SceneModel::~SceneModel()
{
    delete m_impl;
}

bool SceneModel::objectIsExpanded(SceneObject *)
{
    return true;
}

void SceneModel::setObjectIsExpanded(SceneObject *, bool value)
{

}

QModelIndex SceneModel::indexForObject(SceneObject *object)
{
    if(object == nullptr)
        return QModelIndex();
    if(object->parentSceneObject())
    {
        return index(object->index(),0, indexForObject(object->parentSceneObject()));
    } else {
        return index(object->index(),0);
    }

}

SceneObject *SceneModel::objectForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    else
    {
        SceneObject *item = static_cast<SceneObject*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_impl->rootObject;
}


QStringList SceneModel::mimeTypes() const
{
    return QStringList() << SceneObject::SceneObjectMime;
}

bool SceneModel::canDropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    if(mimeData->hasImage() || mimeData->hasUrls())
    {
        return true;
    }
    return QAbstractItemModel::canDropMimeData(mimeData, action, row, column, parent);
}

//receives a list of model indexes list
QMimeData *SceneModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray data; //a kind of RAW format for datas

    QDataStream stream(&data, QIODevice::WriteOnly);

    QVector<SceneObject*> items;

    foreach (const QModelIndex &index, indexes) {
        SceneObject *node = objectForIndex(index);
        //qDebug() << index.row() << node->name();
        if (!items.contains(node))
            items << node;
    }
    int count = items.count();
    stream << QCoreApplication::applicationPid();
    stream << count;
    for(SceneObject *item : items) {
        stream << reinterpret_cast<qlonglong>(item);
    }

    mimeData->setData(SceneObject::SceneObjectMime, data);
    return mimeData;
}

bool SceneModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action)
    Q_UNUSED(column);


    if (mimeData->hasFormat(SceneObject::SceneObjectMime)) {

        QByteArray data = mimeData->data(SceneObject::SceneObjectMime);
        QDataStream stream(&data, QIODevice::ReadOnly);
        qint64 senderPid;
        stream >> senderPid;
        if (senderPid != QCoreApplication::applicationPid())
            return false;

        SceneObject *parentLayer = nullptr;
        if(parent.isValid())
            parentLayer = objectForIndex(parent);

        if(!parentLayer)
            parentLayer = m_impl->rootObject;

        int count;
        stream >> count;
        if (row == -1) {
            if (parent.isValid())
                row = 0;
            else
                row = rowCount(parent);
        }

        QVector<SceneObject*> layers;
        for (int i = 0; i < count; ++i) {
            // Decode data from the QMimeData
            qlonglong objPtr;
            stream >> objPtr;
            SceneObject *node = reinterpret_cast<SceneObject*>(objPtr);

                layers.append(static_cast<SceneObject*>(node));
        }

        //std::sort(layers.begin(), layers.end(),[](LayerPtr layerA, LayerPtr layerB){return layerA->index() < layerB->index();});

        if(parentLayer)
            layers.append(parentLayer);
        //decoApp->undo()->startCapture(new LayerOrderUndo(layers));
        if(parentLayer)
            layers.pop_back();

        QModelIndexList movedIndices;
        for(auto layer : layers)
        {
            //if(row >= layer->index() && parentLayer == layer->parentSceneObject())
            //    --row;

            //qDebug() << "Move Layer:" << layer->name();

            if(parentLayer && layer->parentSceneObject() == parentLayer)
            {
                parentLayer->moveChildToIndex(layer, row);
                movedIndices.append(indexForObject(layer));

                continue;
            }
            else
                layer->setParentSceneObject(parentLayer, row);
            movedIndices.append(indexForObject(layer));

            ++row;
        }

        emit selectionMoved(movedIndices);

    }

    return true;
}

SceneObject *SceneModel::rootObject() const
{
    return m_impl->rootObject;
}

Qt::DropActions SceneModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions SceneModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

Qt::ItemFlags SceneModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    if(index.column() == 0)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

//! [2]
int SceneModel::columnCount(const QModelIndex & /* parent */) const
{
    return 4;
}
//! [2]

QVariant SceneModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_impl->rootObject)
        return QVariant();

    switch(role)
    {
        case Qt::ToolTipRole:
        {
            SceneObject *layer = objectForIndex(index);
            if(layer)
            {
                return layer->name();
            }
        }
            break;

        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            SceneObject *layer = objectForIndex(index);
            if(layer)
            {
                auto fixture = dynamic_cast<DMXReceiver*>(layer);
                if(fixture)
                {
                    switch(index.column())
                    {
                        default:
                            return "";
                        case 0:
                            return layer->name();
                        case 1:
                            return fixture->universe();
                        case 2:
                            return QString::number(fixture->dmxOffset()) + "-" + QString::number(fixture->dmxOffset() + fixture->dmxSize());
                        case 3:
                            return fixture->dmxSize();
                    }
                }
                else
                {
                    if(index.column() == 0)
                        return layer->name();
                    return "";
                }


            }
        }
            break;

        case Qt::SizeHintRole:
            //if(index.column() > 0 && index.column() <= m_channelCount )
                return QSize{32,32};
            //break;
    }

    return QVariant();
}

QVariant SceneModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
            default:
                return "";
            case 0:
                return "Name";
            case 1:
                return "Uni";
            case 2:
                return "Range";
            case 3:
                return "Channels";
        }

    }

    if(role == Qt::ToolTipRole)
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
QModelIndex SceneModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SceneObject *parentLayer = objectForIndex(parent);
    if(!parentLayer)
        return createIndex(row, column, m_impl->rootObject->childAtIndex(row));
    SceneObject *childLayer = parentLayer->childAtIndex(row);
    if (childLayer)
        return createIndex(row, column, childLayer);
    else
        return QModelIndex();

}
//! [6]

//! [7]
QModelIndex SceneModel::parent(const QModelIndex &index) const
{
    SceneObject *childLayer = objectForIndex(index);
    if (!childLayer)
        return QModelIndex();

    SceneObject *parentLayer = childLayer->parentSceneObject();
    if (!parentLayer)
        return QModelIndex();

    return createIndex(parentLayer->index(), 0, parentLayer);
}
//! [7]

//! [8]
int SceneModel::rowCount(const QModelIndex &parent) const
{
    if(!m_impl->rootObject)return 0;

    if (parent.column() > 0)
        return 0;


    SceneObject *parentLayer = objectForIndex(parent);
    if(parentLayer)
        return parentLayer->childCount();
    else
    {
        return m_impl->rootObject->childCount();
    }
}
//! [8]

bool SceneModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if(index.column() == 0)
    {
        objectForIndex(index)->setName(value.toString());
        emit dataChanged(index,index);
        return true;
    }

    return false;
}

void SceneModel::objectWasUpdated(photon::SceneObject *t_object)
{
    QModelIndex index = indexForObject(t_object);
    dataChanged(createIndex(index.row(),0,t_object),createIndex(index.row(),3,t_object));
}

void SceneModel::childWillBeAdded(photon::SceneObject *t_parent, photon::SceneObject *t_child)
{
    beginInsertRows(indexForObject(t_parent), t_child->index(), t_child->index());

    connect(t_child, &SceneObject::childWillBeAdded, this, &SceneModel::childWillBeAdded);
    connect(t_child, &SceneObject::childWasAdded, this, &SceneModel::childWasAdded);
    connect(t_child, &SceneObject::childWillBeRemoved, this, &SceneModel::childWillBeRemoved);
    connect(t_child, &SceneObject::childWasRemoved, this, &SceneModel::childWasRemoved);
    connect(t_child, &SceneObject::childWillBeMoved, this, &SceneModel::childWillBeMoved);
    connect(t_child, &SceneObject::childWasMoved, this, &SceneModel::childWasMoved);
    connect(t_child, &SceneObject::metadataChanged, this, &SceneModel::objectWasUpdated);
}

void SceneModel::childWasAdded(photon::SceneObject *)
{
    endInsertRows();
}

void SceneModel::childWillBeRemoved(photon::SceneObject *t_parent, photon::SceneObject *t_child)
{
    disconnect(t_child, &SceneObject::childWillBeAdded, this, &SceneModel::childWillBeAdded);
    disconnect(t_child, &SceneObject::childWasAdded, this, &SceneModel::childWasAdded);
    disconnect(t_child, &SceneObject::childWillBeRemoved, this, &SceneModel::childWillBeRemoved);
    disconnect(t_child, &SceneObject::childWasRemoved, this, &SceneModel::childWasRemoved);
    disconnect(t_child, &SceneObject::childWillBeMoved, this, &SceneModel::childWillBeMoved);
    disconnect(t_child, &SceneObject::childWasMoved, this, &SceneModel::childWasMoved);
    disconnect(t_child, &SceneObject::metadataChanged, this, &SceneModel::objectWasUpdated);
    beginRemoveRows(indexForObject(t_parent), t_child->index(), t_child->index());
}

void SceneModel::childWasRemoved(photon::SceneObject *)
{
    endRemoveRows();
}

void SceneModel::childWillBeMoved(photon::SceneObject *t_child, int t_newIndex)
{
    //qDebug() << t_child->index() << " -> " << t_newIndex;
    beginMoveRows(indexForObject(t_child->parentSceneObject()),t_child->index(), t_child->index(),indexForObject(t_child->parentSceneObject()),t_newIndex);
}

void SceneModel::childWasMoved(photon::SceneObject *)
{
    endMoveRows();
}


} // namespace photon
