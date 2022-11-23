#ifndef ROUTINECOLLECTIONPANEL_P_H
#define ROUTINECOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "routinecollectionpanel.h"

namespace photon {

class RoutineCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class RoutineCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    RoutineCollectionModel(RoutineCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void routineWillBeAdded(photon::Routine *, int);
    void routineWasAdded(photon::Routine *);
    void routineWillBeRemoved(photon::Routine *, int);
    void routineWasRemoved(photon::Routine *);

private:
    RoutineCollection *m_collection;

};

}

#endif // ROUTINECOLLECTIONPANEL_P_H
