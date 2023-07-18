#ifndef PIXELLAYOUTCOLLECTIONPANEL_P_H
#define PIXELLAYOUTCOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "pixellayoutcollectionpanel.h"

namespace photon {

class PixelLayoutCollection;

class PixelLayoutCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class PixelLayoutCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    PixelLayoutCollectionModel(PixelLayoutCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void layoutWillBeAdded(photon::PixelLayout *, int);
    void layoutWasAdded(photon::PixelLayout *);
    void layoutWillBeRemoved(photon::PixelLayout *, int);
    void layoutWasRemoved(photon::PixelLayout *);

private:
    PixelLayoutCollection *m_collection;

};

}


#endif // PIXELLAYOUTCOLLECTIONPANEL_P_H
