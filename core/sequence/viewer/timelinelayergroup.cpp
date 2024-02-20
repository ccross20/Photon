#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include "timelinelayergroup.h"
#include "sequence/layergroup.h"
#include "sequence/cliplayer.h"
#include "sequence/masterlayer.h"
#include "timelinescene.h"
#include "timelinecliplayer.h"
#include "timelinemasterlayer.h"

namespace photon {

class TimelineLayerGroup::Impl
{
public:
    Impl(TimelineLayerGroup *t_facade);
    LayerItem *findLayer(Layer *, QVector<LayerItem*>::const_iterator* t_it= nullptr);
    void removeLayer(Layer *);
    LayerItem * addLayer(Layer *);
    void layoutLayers();
    QVector<LayerItem*> childLayers;
    TimelineLayerGroup *facade;
};

TimelineLayerGroup::Impl::Impl(TimelineLayerGroup *t_facade):facade(t_facade)
{

}

LayerItem *TimelineLayerGroup::Impl::findLayer(Layer *t_layer, QVector<LayerItem*>::const_iterator* t_it)
{
    auto result = std::find_if(childLayers.cbegin(), childLayers.cend(),[t_layer](LayerItem *t_testLayer){
                     return t_testLayer->layer() == t_layer;
                 });
    if(t_it)
        *t_it = result;

    if(result != childLayers.cend())
        return *result;
    return nullptr;
}

LayerItem *TimelineLayerGroup::Impl::addLayer(Layer *t_layer)
{
    auto clipLayer = dynamic_cast<ClipLayer*>(t_layer);
    if(clipLayer)
    {
        TimelineClipLayer *timelineLayer = new TimelineClipLayer(clipLayer);

        childLayers.append(timelineLayer);
        timelineLayer->setParentItem(facade);
        //facade->scene()->addItem(timelineLayer);
        timelineLayer->addedToScene(static_cast<TimelineScene*>(facade->scene()));
        layoutLayers();

        return timelineLayer;
    }

    auto masterLayer = dynamic_cast<MasterLayer*>(t_layer);
    if(masterLayer)
    {
        TimelineMasterLayer *timelineLayer = new TimelineMasterLayer(masterLayer);

        childLayers.append(timelineLayer);
        //facade->scene()->addItem(timelineLayer);
        timelineLayer->addedToScene(static_cast<TimelineScene*>(facade->scene()));
        layoutLayers();

        return timelineLayer;
    }


    return nullptr;
}

void TimelineLayerGroup::Impl::removeLayer(Layer *t_layer)
{
    QVector<LayerItem*>::const_iterator it;
    auto result = findLayer(t_layer, &it);

    if(result)
    {
        childLayers.erase(it);
        //facade->scene()->removeItem(result);
        delete result;
    }
}

void TimelineLayerGroup::Impl::layoutLayers()
{
    int y = 20;
    for(auto layer : childLayers)
    {
        layer->setPos(0, y);
        y += layer->layer()->height() + 2;
    }
}

TimelineLayerGroup::TimelineLayerGroup(LayerGroup *t_group): LayerItem(t_group),m_impl(new Impl(this))
{
    connect(t_group, &LayerGroup::layerAdded,this,&TimelineLayerGroup::layerAdded);
    connect(t_group, &LayerGroup::layerRemoved,this,&TimelineLayerGroup::layerRemoved);
    connect(t_group, &LayerGroup::layerUpdated,this,&TimelineLayerGroup::layerModified);
}

TimelineLayerGroup::~TimelineLayerGroup()
{
    delete m_impl;
}

QRectF TimelineLayerGroup::boundingRect() const
{
    return QRectF(0,0,300, layer()->height());
}

void TimelineLayerGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    //painter->fillRect(boundingRect(), QColor(40,40,40));
}

void TimelineLayerGroup::layerAdded(photon::Layer *t_layer)
{
    qDebug() << "Layer added " << t_layer->name();
    m_impl->addLayer(t_layer);
}

void TimelineLayerGroup::layerRemoved(photon::Layer *t_layer)
{
    m_impl->removeLayer(t_layer);
}

void TimelineLayerGroup::layerModified(photon::Layer *t_layer)
{

}

void TimelineLayerGroup::removeLayer()
{

}

LayerItem *TimelineLayerGroup::layerAtY(double t_y) const
{
    for(auto layer : m_impl->childLayers)
    {
        //qDebug() << layer->boundingRect();
        auto globalRect = layer->mapRectToScene(layer->boundingRect());
        if(globalRect.top() < t_y && globalRect.bottom() > t_y)
        {
            return layer;
        }
    }
    return nullptr;
}

LayerItem *TimelineLayerGroup::findLayer(Layer *t_layer)
{
    QVector<LayerItem*>::const_iterator it;
    return m_impl->findLayer(t_layer, &it);
}

void TimelineLayerGroup::addedToScene(TimelineScene *t_scene)
{
    for(Layer *layer : static_cast<LayerGroup*>(layer())->layers())
    {
        m_impl->addLayer(layer);
    }
}

void TimelineLayerGroup::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsObject::contextMenuEvent(event);
    if(event->isAccepted())
        return;

    event->accept();


    QMenu menu;

    menu.addAction("Add Layer to Group",this,[this](){
        static_cast<LayerGroup*>(layer())->addLayer(new ClipLayer("Layer " + QString::number(layer()->children().length() + 1)));

    });

    menu.exec(event->screenPos());
}

} // namespace photon
