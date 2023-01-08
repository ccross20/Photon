#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "timelinescene.h"
#include "sequence/sequence.h"
#include "sequence/cliplayer.h"
#include "sequence/canvaslayergroup.h"
#include "sequence/masterlayer.h"
#include "timelinecliplayer.h"
#include "timelinemasterlayer.h"
#include "timelinelayergroup.h"

namespace photon {

class TimelineScene::Impl
{
public:
    Impl(TimelineScene *t_facade, Sequence *t_sequence);
    LayerItem *addLayer(Layer *);
    LayerItem *findLayer(Layer *);
    void layoutLayers();

    Sequence *sequence = nullptr;
    QVector<LayerItem*> layers;
    TimelineScene *facade;
};

TimelineScene::Impl::Impl(TimelineScene *t_facade, Sequence *t_sequence):facade(t_facade)
{

}

LayerItem *TimelineScene::Impl::findLayer(Layer *t_layer)
{
    auto result = std::find_if(layers.cbegin(), layers.cend(),[t_layer](const LayerItem *t_testLayer){
                     return t_testLayer->layer() == t_layer;
                 });
    if(result != layers.cend())
        return *result;
    return nullptr;
}

LayerItem *TimelineScene::Impl::addLayer(Layer *t_layer)
{
    auto clipLayer = dynamic_cast<ClipLayer*>(t_layer);
    if(clipLayer)
    {
        TimelineClipLayer *timelineLayer = new TimelineClipLayer(clipLayer);

        layers.append(timelineLayer);
        facade->addItem(timelineLayer);
        timelineLayer->addedToScene(facade);

        return timelineLayer;
    }

    auto masterLayer = dynamic_cast<MasterLayer*>(t_layer);
    if(masterLayer)
    {
        TimelineMasterLayer *timelineLayer = new TimelineMasterLayer(masterLayer);

        layers.append(timelineLayer);
        facade->addItem(timelineLayer);
        timelineLayer->addedToScene(facade);

        return timelineLayer;
    }

    auto canvasGroup = dynamic_cast<CanvasLayerGroup*>(t_layer);
    if(canvasGroup)
    {
        TimelineLayerGroup *timelineLayer = new TimelineLayerGroup(canvasGroup);

        layers.append(timelineLayer);
        facade->addItem(timelineLayer);
        timelineLayer->addedToScene(facade);

        return timelineLayer;
    }


    return nullptr;

}

void TimelineScene::Impl::layoutLayers()
{
    int y = 0;
    for(auto layer : layers)
    {
        layer->setPos(0, y);
        y += layer->layer()->height() + 2;
    }
}

TimelineScene::TimelineScene(Sequence *t_sequence):QGraphicsScene(),m_impl(new Impl(this, t_sequence))
{
    //setSceneRect(0,0,300,100);
    setSequence(t_sequence);
}
TimelineScene::~TimelineScene()
{
    delete m_impl;
}

void TimelineScene::setSequence(Sequence *t_sequence)
{
    if(m_impl->sequence == t_sequence)
        return;

    clear();
    m_impl->sequence = t_sequence;

    for(Layer *layer : t_sequence->layers())
    {
        if(!m_impl->findLayer(layer))
            m_impl->addLayer(layer);
    }

    m_impl->layoutLayers();

    if(!t_sequence)
        return;

    connect(m_impl->sequence, &Sequence::layerAdded, this, &TimelineScene::layerAdded);
    connect(m_impl->sequence, &Sequence::layerRemoved, this, &TimelineScene::layerRemoved);
}

Sequence *TimelineScene::sequence() const
{
    return m_impl->sequence;
}

void TimelineScene::layerAdded(photon::Layer* t_layer)
{
    if(!m_impl->findLayer(t_layer))
        m_impl->addLayer(t_layer);
    m_impl->layoutLayers();
}

void TimelineScene::layerRemoved(photon::Layer* t_layer)
{
    auto foundLayer = m_impl->findLayer(t_layer);

    if(!foundLayer)
        return;

    removeItem(foundLayer);
    m_impl->layers.removeOne(foundLayer);
    delete foundLayer;
    m_impl->layoutLayers();
}

void TimelineScene::createLayer()
{
    ClipLayer *layer = new ClipLayer;
    layer->setName("Layer " + QString::number(m_impl->sequence->layers().length()+1));
    m_impl->sequence->addLayer(layer);
}

void TimelineScene::createCanvasLayerGroup()
{
    CanvasLayerGroup *layer = new CanvasLayerGroup(nullptr, "Canvas Group");
    m_impl->sequence->addLayer(layer);
}

void TimelineScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    QGraphicsScene::contextMenuEvent(contextMenuEvent);

    if(contextMenuEvent->isAccepted())
        return;

    QMenu menu;
    QAction *createLayer = menu.addAction("Create Empty Layer");
    connect(createLayer, &QAction::triggered, this, &TimelineScene::createLayer);

    QAction *createCanvasLayer = menu.addAction("Create Canvas Layer Group");
    connect(createCanvasLayer, &QAction::triggered, this, &TimelineScene::createCanvasLayerGroup);

    menu.addSeparator();

    menu.addAction("Create Master Curve",[this](){
        MasterLayer *layer = new MasterLayer("Master Curve");
        m_impl->sequence->addLayer(layer);
    });

    menu.exec(contextMenuEvent->screenPos());
}


} // namespace photon
