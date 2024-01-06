#ifndef TAGCOLLECTIONPANEL_P_H
#define TAGCOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "tagcollectionpanel.h"

namespace photon {

class TagCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
};

class TagCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    TagCollectionModel(TagCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void tagWillBeAdded(QString, int);
    void tagWasAdded(QString);
    void tagWillBeRemoved(QString, int);
    void tagWasRemoved(QString);

private:
    TagCollection *m_collection;

};

}

#endif // TAGCOLLECTIONPANEL_P_H
