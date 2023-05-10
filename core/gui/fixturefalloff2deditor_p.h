#ifndef FIXTUREFALLOFF2DEDITOR_P_H
#define FIXTUREFALLOFF2DEDITOR_P_H

#include <QAbstractItemModel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTreeView>
#include "fixturefalloff2deditor.h"
#include "scene/scenemodel.h"

namespace photon {


class GizmoItem : public QGraphicsItem
{
public:
    GizmoItem();
     QRectF boundingRect() const override;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
     QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

class SceneObjectItem : public QGraphicsItem
{
public:
    SceneObjectItem(SceneObject *);
     QRectF boundingRect() const override;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

     SceneObject *sceneObject() const{return m_sceneObject;}

protected:
     QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
     SceneObject *m_sceneObject;
};

class FalloffScene : public QGraphicsScene
{
public:
    FalloffScene();

    GizmoItem *startGizmo() const{return m_startGizmo;}
    GizmoItem *endGizmo() const{return m_endGizmo;}
    void addSceneObjectItem(SceneObjectItem *);
    void removeSceneObjectItem(SceneObjectItem *);

    const QVector<SceneObjectItem*> &sceneObjectItems() const{return m_sceneObjects;}

private:
    QVector<SceneObjectItem*> m_sceneObjects;
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
    QVector<SceneObjectItem*> m_draggingFixtures;
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
    QTreeView *m_treeView;
    SceneModel *m_sceneModel;
    FixtureFalloff2DEditor *m_facade;
};

}

#endif // FIXTUREFALLOFF2DEDITOR_P_H
