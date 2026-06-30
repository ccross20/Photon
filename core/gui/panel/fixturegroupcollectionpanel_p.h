#ifndef FIXTUREGROUPCOLLECTIONPANEL_P_H
#define FIXTUREGROUPCOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include <QAbstractItemModel>
#include "fixturegroupcollectionpanel.h"

namespace photon {

class FixtureGroupCollection;

class FixtureGroupCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class FixtureGroupCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    FixtureGroupCollectionModel(FixtureGroupCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

private slots:
    void groupWillBeAdded(int);
    void groupWasAdded(int);
    void groupWillBeRemoved(int);
    void groupWasRemoved(int);
    void groupChanged(int);

private:
    FixtureGroupCollection *m_collection;
};

}

#endif // FIXTUREGROUPCOLLECTIONPANEL_P_H
