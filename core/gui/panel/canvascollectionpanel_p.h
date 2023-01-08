#ifndef CANVASCOLLECTIONPANEL_P_H
#define CANVASCOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "canvascollectionpanel.h"

namespace photon {

class CanvasCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class CanvasCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    CanvasCollectionModel(CanvasCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void canvasWillBeAdded(photon::Canvas *, int);
    void canvasWasAdded(photon::Canvas *);
    void canvasWillBeRemoved(photon::Canvas *, int);
    void canvasWasRemoved(photon::Canvas *);

private:
    CanvasCollection *m_collection;

};

}


#endif // CANVASCOLLECTIONPANEL_P_H
