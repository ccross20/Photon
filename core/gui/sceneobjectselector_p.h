#ifndef SCENEOBJECTSELECTOR_P_H
#define SCENEOBJECTSELECTOR_P_H

#include <QTreeView>
#include <QPushButton>
#include "sceneobjectselector.h"
#include "scene/scenemodel.h"

namespace photon {

class SelectorItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    SelectorItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class SelectorModel : public SceneModel
{
    Q_OBJECT
public:
    SelectorModel(SceneObject *root, QObject *parent = nullptr);
    QVariant headerData(int section, Qt::Orientation orientation,
                int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    void setSelectedObjects(const QVector<SceneObject *> &t_selected);
    const QVector<SceneObject *> &selectedObjects() const;

public slots:
    void selectAll();
    void selectNone();

signals:
    void selectionChanged();

private:
    bool hasAncestorSelected(SceneObject *) const;
    bool hasSomeDescendantSelected(SceneObject *) const;
    void sortByTreeOrder();
    void collectInTreeOrder(const QModelIndex &parent,
                            const QSet<SceneObject *> &selected,
                            QVector<SceneObject *> &result) const;
    void collectAll(const QModelIndex &parent, QVector<SceneObject *> &result) const;
    void emitDataChangedForAll(const QModelIndex &parent);

    QVector<SceneObject*> m_selectedObjects;
};

class SceneObjectSelector::Impl
{
public:
    Impl(SceneObjectSelector *, SceneObject *);
    SelectorModel  *model;
    QTreeView      *view;
    QPushButton    *selectAllButton;
    QPushButton    *selectNoneButton;
    SceneObjectSelector *facade;
};

}

#endif // SCENEOBJECTSELECTOR_P_H
