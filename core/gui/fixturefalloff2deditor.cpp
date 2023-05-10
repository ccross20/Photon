#include <QPainter>
#include <QHBoxLayout>
#include <QMimeData>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QList>
#include "fixturefalloff2deditor_p.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"
#include "scene/scenemanager.h"

namespace photon {





FalloffScene::FalloffScene() : QGraphicsScene()
{
    m_startGizmo = new GizmoItem;
    addItem(m_startGizmo);
    m_startGizmo->setPos(0,.5);

    m_endGizmo = new GizmoItem;
    addItem(m_endGizmo);
    m_endGizmo->setPos(1.0,.5);
}

void FalloffScene::addSceneObjectItem(SceneObjectItem *t_item)
{
    m_sceneObjects.append(t_item);
    addItem(t_item);
}

void FalloffScene::removeSceneObjectItem(SceneObjectItem *t_item)
{
    m_sceneObjects.removeOne(t_item);
}


FalloffView::FalloffView() : QGraphicsView()
{
    setDragMode(DragMode::RubberBandDrag);
    setViewportUpdateMode(FullViewportUpdate);

    QPixmap p(40,40);
    p.fill(QColor(50,50,50));

    QPainter pPainter(&p);
    pPainter.setPen(Qt::NoPen);
    pPainter.setBrush(QColor(80,80,80));
    pPainter.drawRect(0,20,40,1);
    pPainter.drawRect(20,0,1,40);
    m_gridBrush = QBrush{p};
}

void FalloffView::drawBackground(QPainter *painter, const QRectF &rect)
{
    m_gridBrush.setTransform(transform().inverted());

    painter->setBrush(m_gridBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    auto startPos = static_cast<FalloffScene *>(scene())->startGizmo()->pos();
    auto endPos = static_cast<FalloffScene *>(scene())->endGizmo()->pos();

    QLinearGradient gradient(startPos, endPos);
    gradient.setColorAt(0, QColor(80,80,80));
    gradient.setColorAt(1, Qt::white);

    QBrush gradientBrush{gradient};

    painter->fillRect(rect, gradientBrush);
}

void FalloffView::dragEnterEvent(QDragEnterEvent *event)
{

    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasFormat(Fixture::SceneObjectMime))
    {
        event->acceptProposedAction();

        QByteArray data = mimeData->data(SceneObject::SceneObjectMime);
        QDataStream stream(&data, QIODevice::ReadOnly);
        qint64 senderPid;
        stream >> senderPid;

        if (senderPid != QCoreApplication::applicationPid())
            return;

        QList<qlonglong> fixtures;
        stream >> fixtures;

        for(auto fixture : fixtures)
        {
            SceneObjectItem *item = new SceneObjectItem(reinterpret_cast<SceneObject *>(fixture));

            static_cast<FalloffScene *>(scene())->addSceneObjectItem(item);
            item->setPos(mapToScene(event->position().toPoint()));
            m_draggingFixtures.append(item);
        }
        m_lastPosition = mapToScene(event->position().toPoint());

    } else {
        QGraphicsView::dragEnterEvent(event);
    }

}

void FalloffView::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->mimeData()->hasFormat(SceneObject::SceneObjectMime))
    {
        QPointF delta =  mapToScene(event->position().toPoint()) - m_lastPosition;
        for(auto item : m_draggingFixtures)
        {
            item->moveBy(delta.x(), delta.y());
        }

        m_lastPosition = mapToScene(event->position().toPoint());
        event->acceptProposedAction();
    }
    else {
        QGraphicsView::dragMoveEvent(event);
    }
}

void FalloffView::dragLeaveEvent(QDragLeaveEvent *event)
{
    for(auto item : m_draggingFixtures)
    {        
        static_cast<FalloffScene *>(scene())->removeSceneObjectItem(item);
        delete item;
    }

    m_draggingFixtures.clear();
     QGraphicsView::dragLeaveEvent(event);
}

void FalloffView::dropEvent(QDropEvent *event)
{
    m_draggingFixtures.clear();

}

SceneObjectItem::SceneObjectItem(SceneObject *t_sceneObj) : QGraphicsItem(),m_sceneObject(t_sceneObj)
{
    setFlags(GraphicsItemFlag::ItemIsSelectable | GraphicsItemFlag::ItemIsMovable | GraphicsItemFlag::ItemSendsScenePositionChanges);
    setScale(.002);

}

QRectF SceneObjectItem::boundingRect() const
{
    return QRectF(-25,-25,50,50);
}

void SceneObjectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->fillRect(-24,-24,48,48, isSelected() ? Qt::cyan : Qt::red);
}

QVariant SceneObjectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        QRectF rect = QRectF(0,0,1,1);
        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}



GizmoItem::GizmoItem() : QGraphicsItem()
{
    setFlags(GraphicsItemFlag::ItemIsSelectable | GraphicsItemFlag::ItemIsMovable | GraphicsItemFlag::ItemSendsScenePositionChanges);
    setScale(.002);
}

QRectF GizmoItem::boundingRect() const
{
    return QRectF(-10,-10,20,20);
}

QVariant GizmoItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        QRectF rect = QRectF(0,0,1,1);
        if (!rect.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void GizmoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::black);
    painter->drawEllipse(QPoint(0,0),8,8);
}


FixtureFalloff2DEditor::Impl::Impl(FixtureFalloff2DEditor* t_facade) : m_facade(t_facade)
{
    QHBoxLayout *hLayout = new QHBoxLayout;

    m_scene = new FalloffScene;
    m_view = new FalloffView;
    m_view->setScene(m_scene);
    m_view->fitInView(QRect(0,0,1,1));
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scene->setSceneRect(QRect(0,0,1,1));
    hLayout->addWidget(m_view);


    m_sceneModel = new SceneModel(photonApp->project()->sceneRoot());

    m_treeView = new QTreeView;
    m_treeView->setDragEnabled(true);
    m_treeView->setDragDropMode(QAbstractItemView::DragOnly);
    m_treeView->setMaximumWidth(200);
    m_treeView->setModel(m_sceneModel);
    hLayout->addWidget(m_treeView);

    m_facade->setLayout(hLayout);
}

FixtureFalloff2DEditor::FixtureFalloff2DEditor() : QWidget(),m_impl(new Impl(this))
{
    connect(m_impl->m_scene, &FalloffScene::changed, this, &FixtureFalloff2DEditor::sceneChanged);
}
FixtureFalloff2DEditor::~FixtureFalloff2DEditor()
{
    delete m_impl;
}

void FixtureFalloff2DEditor::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);
    //m_impl->m_scene->setSceneRect(QRect(0,0,1,1));
    m_impl->m_view->fitInView(QRect(0,0,1,1));

}

void FixtureFalloff2DEditor::sceneChanged(const QList<QRectF> &regions)
{
    FalloffMap2D map;
    map.setStartPosition(m_impl->m_scene->startGizmo()->pos());
    map.setEndPosition(m_impl->m_scene->endGizmo()->pos());

    for(auto item : m_impl->m_scene->sceneObjectItems())
    {
        map.addSceneObject(item->sceneObject(), item->pos());
    }

    m_impl->m_map = map;

    emit mapUpdated();
}

void FixtureFalloff2DEditor::setMap(const FalloffMap2D &t_map)
{
    m_impl->m_map = t_map;

    for(const auto &sceneObject : t_map.sceneObjectData())
    {
        auto fix = photonApp->project()->scene()->findObjectById(sceneObject.sceneObjectId);
        if(fix)
        {
            auto item = new SceneObjectItem(fix);
            item->setPos(sceneObject.position);
            m_impl->m_scene->addSceneObjectItem(item);
        }
    }

    m_impl->m_scene->startGizmo()->setPos(t_map.startPosition());
    m_impl->m_scene->endGizmo()->setPos(t_map.endPosition());
}

FalloffMap2D FixtureFalloff2DEditor::map() const
{
    return m_impl->m_map;
}

} // namespace photon
