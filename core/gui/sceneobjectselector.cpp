#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include "sceneobjectselector_p.h"
#include "scene/sceneobject.h"

namespace photon {


SelectorItemDelegate::SelectorItemDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{

}

bool SelectorItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{


        if(index.column() == 0)
        {
            if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
            {
                model->setData(index,index.data().toInt() == 1 ? 0 : 1);


                QModelIndexList children;
                getAllChildren(index, children, model);


                for(const QModelIndex &idx : children)
                    static_cast<SelectorModel*>(model)->updateIndex(idx);


                return true;
            }
        }



    return false;
}

void SelectorItemDelegate::getAllChildren(const QModelIndex &index, QModelIndexList &indicies, QAbstractItemModel *model)
{
    indicies.push_back(index);
    for (int i = 0; i != model->rowCount(index); ++i)
        getAllChildren(model->index(i,0,index), indicies, model);
        //getAllChildren(index.child(i,0), indicies, model);
}

void SelectorItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{


    //painter->fillRect(option.rect.adjusted(1,1,-1,-1), QBrush(Qt::red));

    QRect iconRect = QRect(option.rect.topLeft(),option.rect.size()).adjusted(2,2,-2,-2);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if(index.column() == 0)
    {
        if(index.data().toInt() == 1)
        {
            painter->fillRect(iconRect, Qt::green);
        }
        else if(index.data().toInt() == 2)
        {
            painter->fillRect(iconRect, Qt::gray);
        }
        else
        {
            painter->setPen(Qt::NoPen);
            painter->fillRect(iconRect, Qt::white);
        }
    }
    else if(index.column() == 1)
    {
        if(option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        painter->setBrush(Qt::NoBrush);
        if (option.state & QStyle::State_Selected)
            painter->setPen(QPen(option.palette.highlightedText().color()));
        else
            painter->setPen(QPen(index.data(Qt::ForegroundRole).value<QColor>()));
            //painter->setPen(QPen(option.palette.text().color()));

        painter->drawText(option.rect,index.data().toString());
    }

    painter->restore();
}

QSize SelectorItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
        return QSize(32,32);
}




SelectorModel::SelectorModel(SceneObject *t_root, QObject *t_parent) : SceneModel(t_root, t_parent)
{

}

QVariant SelectorModel::headerData(int section, Qt::Orientation orientation,
            int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(section == 1)
            return "Name";

        return "";
    }

    if(role == Qt::ToolTipRole)
    {
        if(section == 1)
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

QVariant SelectorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !rootObject())
        return QVariant();

    switch(role)
    {
        case Qt::ToolTipRole:
        {
            SceneObject *layer = objectForIndex(index);
            if(layer)
            {
                if(index.column() == 0)
                    return "Is Selected";
                return layer->name();
            }
        }
            break;

        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            SceneObject *sceneObj = objectForIndex(index);
            if(sceneObj)
            {
                if(index.column() == 0)
                {
                    if(hasAncestorSelected(sceneObj))
                        return 2;
                    if(m_selectedObjects.contains(sceneObj))
                            return 1;
                    return 0;
                }
                return sceneObj->name();
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

bool SelectorModel::hasAncestorSelected(SceneObject *t_obj) const
{
    SceneObject *par = t_obj->parentSceneObject();
    while(par)
    {
        if(m_selectedObjects.contains(par))
            return true;
        par = par->parentSceneObject();
    }
    return false;
}

int SelectorModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

bool SelectorModel::setData(const QModelIndex &index, const QVariant &value,
             int role)
{
    if (role != Qt::EditRole)
        return false;

    if(index.column() == 0)
    {
        if(hasAncestorSelected(objectForIndex(index)))
            return false;
        if(value.toBool())
            m_selectedObjects.append(objectForIndex(index));
        else
            m_selectedObjects.removeOne(objectForIndex(index));
        emit selectionChanged();
        return true;
    }
    return false;
}

void SelectorModel::updateIndex(const QModelIndex &index)
{
    emit dataChanged(index, index);
}
void SelectorModel::setSelectedObjects(const QVector<SceneObject *> &t_selected)
{
    beginResetModel();
    m_selectedObjects = t_selected;
    endResetModel();
}

const QVector<SceneObject *> &SelectorModel::selectedObjects() const
{
    return m_selectedObjects;
}


SceneObjectSelector::Impl::Impl(SceneObjectSelector *t_facade, SceneObject *t_root):facade(t_facade)
{
    model = new SelectorModel(t_root, facade);
    view = new QTreeView;
    view->setModel(model);
    view->setItemDelegate(new SelectorItemDelegate);
}

SceneObjectSelector::SceneObjectSelector(SceneObject *t_root, QWidget *parent)
    : QWidget{parent},m_impl(new Impl(this, t_root))
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->view);
    setLayout(vLayout);

    connect(m_impl->model, &SelectorModel::selectionChanged, this, &SceneObjectSelector::selectionChanged);
}

SceneObjectSelector::~SceneObjectSelector()
{
    delete m_impl;
}

void SceneObjectSelector::setSelectedObjects(const QVector<SceneObject *> &t_selected)
{
    m_impl->model->setSelectedObjects(t_selected);
}

QVector<SceneObject *> SceneObjectSelector::selectedObjects() const
{
    return m_impl->model->selectedObjects();
}

} // namespace photon
