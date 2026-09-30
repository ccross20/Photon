#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "timelinescene.h"
#include "sequence/sequence.h"
#include "sequence/cliplayer.h"
#include "timelinecliplayer.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/canvas.h"
#include "sequence/clip.h"
#include "sequence/clipclipboard.h"
#include "sequenceclip.h"

namespace photon {

class TimelineScene::Impl
{
public:
    Impl(TimelineScene *t_facade, Sequence *t_sequence);
    LayerItem *addLayer(Layer *);
    LayerItem *findLayer(Layer *);
    void layoutLayers();

    Sequence *sequence = nullptr;
    ClipLayer *activeLayer = nullptr;
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
    setSequence(t_sequence);
    //setSceneRect(QRectF{-100000,-1000,20000000,20000});

}
TimelineScene::~TimelineScene()
{
    delete m_impl;
}

LayerItem *TimelineScene::itemForLayer(Layer* t_layer) const
{
    return m_impl->findLayer(t_layer);
}

SequenceClip *TimelineScene::itemForClip(Clip* t_clip) const
{
    auto layer = itemForLayer(t_clip->layer());
    if(layer)
        return static_cast<TimelineClipLayer*>(layer)->itemForClip(t_clip);

    return nullptr;
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

    for(Layer *layer : t_sequence->layers())
    {
        if(auto *clipLayer = dynamic_cast<ClipLayer*>(layer))
        {
            setActiveLayer(clipLayer);
            break;
        }
    }

    connect(m_impl->sequence, &Sequence::layerAdded, this, &TimelineScene::layerAdded);
    connect(m_impl->sequence, &Sequence::layerRemoved, this, &TimelineScene::layerRemoved);


    //addRect(QRect{0,0,100,100},Qt::NoPen, Qt::red);
    //addRect(QRect{300,300,100,100},Qt::NoPen, Qt::red);
}

Sequence *TimelineScene::sequence() const
{
    return m_impl->sequence;
}

ClipLayer *TimelineScene::activeLayer() const
{
    return m_impl->activeLayer;
}

QVector<Clip*> TimelineScene::selectedClips() const
{
    QVector<Clip*> clips;
    for(auto *item : selectedItems())
        if(auto *clipItem = dynamic_cast<SequenceClip*>(item))
            clips.append(clipItem->clip());
    return clips;
}

void TimelineScene::copySelectedClips()
{
    ClipClipboard::copy(selectedClips());
}

void TimelineScene::cutSelectedClips()
{
    const QVector<Clip*> clips = selectedClips();
    if(clips.isEmpty())
        return;
    ClipClipboard::copy(clips);
    for(Clip *clip : clips)
    {
        clip->layer()->removeClip(clip);
        delete clip;
    }
}

void TimelineScene::pasteClips(double t_time, ClipLayer *t_layer)
{
    ClipLayer *target = t_layer ? t_layer : m_impl->activeLayer;
    const QVector<Clip*> pasted = ClipClipboard::paste(m_impl->sequence, target, t_time);
    if(pasted.isEmpty())
        return;

    clearSelection();
    for(Clip *clip : pasted)
        if(auto *item = itemForClip(clip))
            item->setSelected(true);
}

void TimelineScene::setActiveLayer(ClipLayer *t_layer)
{
    if(m_impl->activeLayer == t_layer)
        return;
    m_impl->activeLayer = t_layer;
    update();   // lanes repaint their highlight
    emit activeLayerChanged(t_layer);
}

void TimelineScene::layerAdded(photon::Layer* t_layer)
{
    if(!m_impl->findLayer(t_layer))
        m_impl->addLayer(t_layer);
    m_impl->layoutLayers();
    if(!m_impl->activeLayer)
        setActiveLayer(dynamic_cast<ClipLayer*>(t_layer));
}

LayerItem *TimelineScene::layerAtY(double t_y) const
{
    for(auto layer : m_impl->layers)
    {
        //qDebug() << layer->boundingRect();
        auto globalRect = layer->mapRectToScene(layer->boundingRect());
        if(globalRect.top() < t_y && globalRect.bottom() > t_y)
            return layer;
    }
    return nullptr;
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

    if(t_layer == m_impl->activeLayer)
    {
        ClipLayer *replacement = nullptr;
        for(Layer *layer : m_impl->sequence->layers())
        {
            auto *clipLayer = dynamic_cast<ClipLayer*>(layer);
            if(clipLayer && layer != t_layer)
            {
                replacement = clipLayer;
                break;
            }
        }
        setActiveLayer(replacement);
    }
}

void TimelineScene::createLayer()
{
    ClipLayer *layer = new ClipLayer;
    layer->setName("Layer " + QString::number(m_impl->sequence->layers().length()+1));
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

    menu.exec(contextMenuEvent->screenPos());
}


} // namespace photon
