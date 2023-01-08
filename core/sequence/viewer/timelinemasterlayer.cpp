#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include "timelinemasterlayer.h"
#include "sequence/masterlayer.h"
#include "sequence/sequence.h"

namespace photon {


class TimelineMasterLayer::Impl
{
public:
    Impl(TimelineMasterLayer *t_facade, MasterLayer *t_layer);
    MasterLayer *layer;
    TimelineMasterLayer *facade;
    bool isHovering = false;
};

TimelineMasterLayer::Impl::Impl(TimelineMasterLayer *t_facade, MasterLayer *t_layer):layer(t_layer),facade(t_facade)
{

}



TimelineMasterLayer::TimelineMasterLayer(MasterLayer *t_layer): LayerItem(t_layer),m_impl(new Impl(this, t_layer))
{

    setFlag(QGraphicsItem::ItemIsSelectable);
}

TimelineMasterLayer::~TimelineMasterLayer()
{
    delete m_impl;
}

void TimelineMasterLayer::addedToScene(TimelineScene *t_scene)
{

}

void TimelineMasterLayer::removeLayer()
{
    m_impl->layer->sequence()->removeLayer(m_impl->layer);
}

QRectF TimelineMasterLayer::boundingRect() const
{
    /*
    if(scene())
        return QRectF(0,0,scene()->width(), m_impl->layer->height());

    else
    */
        return QRectF(0,0,300, m_impl->layer->height());

}

void TimelineMasterLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    double xScale = painter->transform().m11();

    painter->scale(1.0/xScale,1.0);

    QRectF scaledRect = boundingRect();
    scaledRect.setWidth(scaledRect.width() * xScale);

    painter->fillRect(scaledRect, m_impl->isHovering ? Qt::yellow : QColor(60,60,80));
    if(isSelected())
    {
        painter->setPen(QPen(Qt::cyan,2));
        painter->drawRect(scaledRect.adjusted(1,1,-1,-1));
    }
    painter->setClipRect(scaledRect);
    painter->drawText(2,20,m_impl->layer->name());
}

void TimelineMasterLayer::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsObject::contextMenuEvent(event);
    /*
    if(event->isAccepted())
        return;

    event->accept();

    QMenu menu;
    QMenu *routineMenu = menu.addMenu("Add Routine");
    double time = event->scenePos().x();

    for(auto routine : photonApp->project()->routines()->routines())
    {
        connect(routineMenu->addAction(routine->name()), &QAction::triggered, this, [routine, time, this](){addRoutine(routine,time);});
    }

    menu.addSeparator();
    QAction *removeLayer = menu.addAction("Remove Layer");
    connect(removeLayer, &QAction::triggered, this, &TimelineClipLayer::removeLayer);

    menu.exec(event->screenPos());
    */
}

} // namespace photon
