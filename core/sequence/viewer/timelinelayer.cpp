#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include "timelinelayer.h"
#include "sequenceclip.h"
#include "sequence/layer.h"
#include "sequence/clip.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "project/project.h"
#include "routine/routinecollection.h"
#include "routine/routine.h"

namespace photon {


class TimelineLayer::Impl
{
public:
    Impl(TimelineLayer *t_facade, Layer *t_layer);
    SequenceClip *findClip(Clip *, QVector<SequenceClip*>::const_iterator* t_it= nullptr);
    void removeClip(Clip *);
    SequenceClip * addClip(Clip *);
    QVector<SequenceClip*> clips;
    Layer *layer;
    TimelineLayer *facade;
};

TimelineLayer::Impl::Impl(TimelineLayer *t_facade, Layer *t_layer):layer(t_layer),facade(t_facade)
{

}

SequenceClip *TimelineLayer::Impl::findClip(Clip *t_clip, QVector<SequenceClip*>::const_iterator* t_it)
{
    auto result = std::find_if(clips.cbegin(), clips.cend(),[t_clip](SequenceClip *t_testClip){
                     return t_testClip->clip() == t_clip;
                 });
    if(t_it)
        *t_it = result;

    if(result != clips.cend())
        return *result;
    return nullptr;
}

SequenceClip *TimelineLayer::Impl::addClip(Clip *t_clip)
{
    SequenceClip *sequenceClip = new SequenceClip(t_clip);
    clips.append(sequenceClip);
    facade->scene()->addItem(sequenceClip);
    sequenceClip->setParentItem(facade);
    sequenceClip->setPos(t_clip->startTime(),0);
    return sequenceClip;
}

void TimelineLayer::Impl::removeClip(Clip *t_clip)
{
    QVector<SequenceClip*>::const_iterator it;
    auto result = findClip(t_clip, &it);

    if(result)
    {
        clips.erase(it);
        //facade->scene()->removeItem(result);
        delete result;
    }
}


TimelineLayer::TimelineLayer(Layer *t_layer): QGraphicsObject(),m_impl(new Impl(this, t_layer))
{
    connect(t_layer, &Layer::clipAdded, this, &TimelineLayer::clipAdded);
    connect(t_layer, &Layer::clipRemoved, this, &TimelineLayer::clipRemoved);
    connect(t_layer, &Layer::clipModified, this, &TimelineLayer::clipModified);
}

TimelineLayer::~TimelineLayer()
{
    delete m_impl;
}

void TimelineLayer::addedToScene(TimelineScene *t_scene)
{
    for(Clip *clip : m_impl->layer->clips())
    {
        m_impl->addClip(clip);
    }
}

void TimelineLayer::addClip(SequenceClip *t_clip)
{
    t_clip->setParentItem(this);
    t_clip->setPos(t_clip->clip()->startTime(),0);
}

void TimelineLayer::clipAdded(photon::Clip *t_clip)
{
    m_impl->addClip(t_clip);
}

void TimelineLayer::clipRemoved(photon::Clip *t_clip)
{
    m_impl->removeClip(t_clip);
}

void TimelineLayer::clipModified(photon::Clip *t_clip)
{
    auto sequenceClip = m_impl->findClip(t_clip);
    if(sequenceClip)
    {
        sequenceClip->setPos(t_clip->startTime(),0);
        sequenceClip->updateDuration();
    }
}

void TimelineLayer::removeLayer()
{
    m_impl->layer->sequence()->removeLayer(m_impl->layer);
}

QRectF TimelineLayer::boundingRect() const
{
    /*
    if(scene())
        return QRectF(0,0,scene()->width(), m_impl->layer->height());

    else
    */
        return QRectF(0,0,300, m_impl->layer->height());

}

void TimelineLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    painter->fillRect(boundingRect(), QColor(40,40,40));
}

Layer *TimelineLayer::layer() const
{
    return m_impl->layer;
}

void TimelineLayer::addRoutine(photon::Routine *t_routine, double t_time)
{
    Clip *clip = new Clip(t_time, 5.0);
    clip->setRoutine(t_routine);
    m_impl->layer->addClip(clip);
}

void TimelineLayer::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsObject::contextMenuEvent(event);
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
    connect(removeLayer, &QAction::triggered, this, &TimelineLayer::removeLayer);

    menu.exec(event->screenPos());
}

} // namespace photon
