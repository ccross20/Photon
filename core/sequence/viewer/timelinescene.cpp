#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "timelinescene.h"
#include "sequence/sequence.h"
#include "sequence/layer.h"
#include "timelinelayer.h"

namespace photon {

class TimelineScene::Impl
{
public:
    Impl(TimelineScene *t_facade, Sequence *t_sequence);
    TimelineLayer *addLayer(Layer *);
    TimelineLayer *findLayer(Layer *);
    void layoutLayers();

    Sequence *sequence = nullptr;
    QVector<TimelineLayer*> layers;
    TimelineScene *facade;
};

TimelineScene::Impl::Impl(TimelineScene *t_facade, Sequence *t_sequence):facade(t_facade)
{

}

TimelineLayer *TimelineScene::Impl::findLayer(Layer *t_layer)
{
    auto result = std::find_if(layers.cbegin(), layers.cend(),[t_layer](const TimelineLayer *t_testLayer){
                     return t_testLayer->layer() == t_layer;
                 });
    if(result != layers.cend())
        return *result;
    return nullptr;
}

TimelineLayer *TimelineScene::Impl::addLayer(Layer *t_layer)
{
    TimelineLayer *timelineLayer = new TimelineLayer(t_layer);

    layers.append(timelineLayer);
    facade->addItem(timelineLayer);
    timelineLayer->addedToScene(facade);

    return timelineLayer;
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
    Layer *layer = new Layer;
    layer->setName("Layer " + QString::number(m_impl->sequence->layers().length()+1));
    m_impl->sequence->addLayer(layer);
}

void TimelineScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    QGraphicsScene::contextMenuEvent(contextMenuEvent);

    if(contextMenuEvent->isAccepted())
        return;

    QMenu menu;
    QAction *createLayer = menu.addAction("Create Layer");
    connect(createLayer, &QAction::triggered, this, &TimelineScene::createLayer);
    menu.exec(contextMenuEvent->screenPos());
}


} // namespace photon
