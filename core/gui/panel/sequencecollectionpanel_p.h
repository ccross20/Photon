#ifndef SEQUENCECOLLECTIONPANEL_P_H
#define SEQUENCECOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "sequencecollectionpanel.h"
namespace photon {

class SequenceCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class SequenceCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    SequenceCollectionModel(SequenceCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void sequenceWillBeAdded(photon::Sequence *, int);
    void sequenceWasAdded(photon::Sequence *, int);
    void sequenceWillBeRemoved(photon::Sequence *, int);
    void sequenceWasRemoved(photon::Sequence *, int);

private:
    SequenceCollection *m_collection;

};

}
#endif // SEQUENCECOLLECTIONPANEL_P_H
