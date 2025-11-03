#ifndef SURFACECOLLECTIONPANEL_P_H
#define SURFACECOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "surfacecollectionpanel.h"
namespace photon {

class SurfaceCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class SurfaceCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    SurfaceCollectionModel(SurfaceCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void surfaceWillBeAdded(photon::Surface *, int);
    void surfaceWasAdded(photon::Surface *, int);
    void surfaceWillBeRemoved(photon::Surface *, int);
    void surfaceWasRemoved(photon::Surface *, int);

private:
    SurfaceCollection *m_collection;

};

}

#endif // SURFACECOLLECTIONPANEL_P_H
