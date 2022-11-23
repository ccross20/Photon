#ifndef FIXTUREFALLOFF2DEDITOR_P_H
#define FIXTUREFALLOFF2DEDITOR_P_H

#include <QAbstractItemModel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QListView>
#include "fixturefalloff2deditor.h"

namespace photon {

class FalloffEditorFixtureModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    FalloffEditorFixtureModel(FixtureCollection *);

    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
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

class GizmoItem : public QGraphicsItem
{
public:
    GizmoItem();
     QRectF boundingRect() const override;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
     QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

class FixtureItem : public QGraphicsItem
{
public:
    FixtureItem(Fixture *);
     QRectF boundingRect() const override;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

     Fixture *fixture() const{return m_fixture;}

protected:
     QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
     Fixture *m_fixture;
};

class FalloffScene : public QGraphicsScene
{
public:
    FalloffScene();

    GizmoItem *startGizmo() const{return m_startGizmo;}
    GizmoItem *endGizmo() const{return m_endGizmo;}
    void addFixtureItem(FixtureItem *);
    void removeFixtureItem(FixtureItem *);

    const QVector<FixtureItem*> &fixtureItems() const{return m_fixtures;}

private:
    QVector<FixtureItem*> m_fixtures;
    GizmoItem *m_startGizmo;
    GizmoItem *m_endGizmo;
};


class FalloffView : public QGraphicsView
{
public:
    FalloffView();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QVector<FixtureItem*> m_draggingFixtures;
    QPointF m_lastPosition;
    QBrush m_gridBrush;
};


class FixtureFalloff2DEditor::Impl
{
public:
    Impl(FixtureFalloff2DEditor*);
    FalloffMap2D m_map;
    FalloffScene *m_scene;
    FalloffView *m_view;
    QListView *m_collectionView;
    FalloffEditorFixtureModel *m_collectionModel;
    FixtureFalloff2DEditor *m_facade;
};

}

#endif // FIXTUREFALLOFF2DEDITOR_P_H
