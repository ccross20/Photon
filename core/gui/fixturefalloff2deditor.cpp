#include <QPainter>
#include <QHBoxLayout>
#include <QMimeData>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QList>
#include "fixturefalloff2deditor_p.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {




FalloffEditorFixtureModel::FalloffEditorFixtureModel(FixtureCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &FixtureCollection::fixtureWillBeAdded, this, &FalloffEditorFixtureModel::fixtureWillBeAdded);
    connect(m_collection, &FixtureCollection::fixtureWasAdded, this, &FalloffEditorFixtureModel::fixtureWasAdded);
    connect(m_collection, &FixtureCollection::fixtureWillBeRemoved, this, &FalloffEditorFixtureModel::fixtureWillBeRemoved);
    connect(m_collection, &FixtureCollection::fixtureWasRemoved, this, &FalloffEditorFixtureModel::fixtureWasRemoved);
}

QStringList FalloffEditorFixtureModel::mimeTypes() const
{
    return QStringList() << Fixture::FixtureMime;
}

Qt::DropActions FalloffEditorFixtureModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::LinkAction;
}

QMimeData *FalloffEditorFixtureModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray data; //a kind of RAW format for datas

    QDataStream stream(&data, QIODevice::WriteOnly);

    QSet<qlonglong> fixtures;

    foreach (const QModelIndex &index, indexes) {
        fixtures.insert(reinterpret_cast<qlonglong>(m_collection->fixtureAtIndex(index.row())));

    }
    stream << QCoreApplication::applicationPid();
    stream << fixtures.values();

    mimeData->setData(Fixture::FixtureMime, data);
    return mimeData;
}

void FalloffEditorFixtureModel::fixtureWillBeAdded(photon::Fixture *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void FalloffEditorFixtureModel::fixtureWasAdded(photon::Fixture *)
{
    endInsertRows();
}

void FalloffEditorFixtureModel::fixtureWillBeRemoved(photon::Fixture *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void FalloffEditorFixtureModel::fixtureWasRemoved(photon::Fixture *)
{
    endRemoveRows();
}

int FalloffEditorFixtureModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

 QModelIndex FalloffEditorFixtureModel::index(int row, int column, const QModelIndex &parent) const
 {
     return createIndex(row, column, m_collection->fixtureAtIndex(row));
 }

 QModelIndex FalloffEditorFixtureModel::parent(const QModelIndex &index) const
 {
     return QModelIndex();
 }

Qt::ItemFlags FalloffEditorFixtureModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

QVariant FalloffEditorFixtureModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
        default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->fixtureAtIndex(index.row())->name();

    }
}

QVariant FalloffEditorFixtureModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        default:
            return QVariant();

        case Qt::DisplayRole:
            return "Name";

    }
}

int FalloffEditorFixtureModel::rowCount(const QModelIndex &) const
{
    return m_collection->fixtureCount();
}


FalloffScene::FalloffScene() : QGraphicsScene()
{
    m_startGizmo = new GizmoItem;
    addItem(m_startGizmo);
    m_startGizmo->setPos(0,.5);

    m_endGizmo = new GizmoItem;
    addItem(m_endGizmo);
    m_endGizmo->setPos(1.0,.5);
}

void FalloffScene::addFixtureItem(FixtureItem *t_item)
{
    m_fixtures.append(t_item);
    addItem(t_item);
}

void FalloffScene::removeFixtureItem(FixtureItem *t_item)
{
    m_fixtures.removeOne(t_item);
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
    if(mimeData->hasFormat(Fixture::FixtureMime))
    {
        event->acceptProposedAction();

        QByteArray data = mimeData->data(Fixture::FixtureMime);
        QDataStream stream(&data, QIODevice::ReadOnly);
        qint64 senderPid;
        stream >> senderPid;

        if (senderPid != QCoreApplication::applicationPid())
            return;

        QList<qlonglong> fixtures;
        stream >> fixtures;

        for(auto fixture : fixtures)
        {
            FixtureItem *item = new FixtureItem(reinterpret_cast<Fixture *>(fixture));

            static_cast<FalloffScene *>(scene())->addFixtureItem(item);
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
    if(event->mimeData()->hasFormat(Fixture::FixtureMime))
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
        static_cast<FalloffScene *>(scene())->removeFixtureItem(item);
        delete item;
    }

    m_draggingFixtures.clear();
     QGraphicsView::dragLeaveEvent(event);
}

void FalloffView::dropEvent(QDropEvent *event)
{
    m_draggingFixtures.clear();

}

FixtureItem::FixtureItem(Fixture *t_fixture) : QGraphicsItem(),m_fixture(t_fixture)
{
    setFlags(GraphicsItemFlag::ItemIsSelectable | GraphicsItemFlag::ItemIsMovable | GraphicsItemFlag::ItemSendsScenePositionChanges);
    setScale(.002);

}

QRectF FixtureItem::boundingRect() const
{
    return QRectF(-25,-25,50,50);
}

void FixtureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->fillRect(-24,-24,48,48, isSelected() ? Qt::cyan : Qt::red);
}

QVariant FixtureItem::itemChange(GraphicsItemChange change, const QVariant &value)
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


    m_collectionModel = new FalloffEditorFixtureModel(photonApp->project()->fixtures());

    m_collectionView = new QListView;
    m_collectionView->setDragEnabled(true);
    m_collectionView->setDragDropMode(QAbstractItemView::DragOnly);
    m_collectionView->setMaximumWidth(200);
    m_collectionView->setModel(m_collectionModel);
    hLayout->addWidget(m_collectionView);

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

    for(auto item : m_impl->m_scene->fixtureItems())
    {
        map.addFixture(item->fixture(), item->pos());
    }

    emit mapUpdated();
}

void FixtureFalloff2DEditor::setMap(const FalloffMap2D &t_map)
{
    m_impl->m_map = t_map;

    for(const auto &fixture : t_map.fixtureData())
    {
        auto item = new FixtureItem(fixture.fixture);
        item->setPos(fixture.position);
        m_impl->m_scene->addFixtureItem(item);
    }

    m_impl->m_scene->startGizmo()->setPos(t_map.startPosition());
    m_impl->m_scene->endGizmo()->setPos(t_map.endPosition());
}

FalloffMap2D FixtureFalloff2DEditor::map() const
{
    return m_impl->m_map;
}

} // namespace photon
