#ifndef FIXTURECOLLECTIONPANEL_P_H
#define FIXTURECOLLECTIONPANEL_P_H

#include <QListView>
#include <QPushButton>
#include "fixturecollectionpanel.h"
#include "fixture/fixtureeditorwidget.h"

namespace photon {

class FixtureCollectionPanel::Impl
{
public:
    QListView *listView;
    QPushButton *addButton;
    QPushButton *removeButton;
    FixtureEditorWidget *editorWidget;
};

class FixtureCollectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    FixtureCollectionModel(FixtureCollection *);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
     QModelIndex parent(const QModelIndex &index) const override;

private slots:

    void fixtureWillBeAdded(photon::Fixture *, int);
    void fixtureWasAdded(photon::Fixture *);
    void fixtureWillBeRemoved(photon::Fixture *, int);
    void fixtureWasRemoved(photon::Fixture *);

private:
    FixtureCollection *m_collection;

};

}

#endif // FIXTURECOLLECTIONPANEL_P_H
