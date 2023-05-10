#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include "timelinecliplayer.h"
#include "sequenceclip.h"
#include "sequence/cliplayer.h"
#include "sequence/routineclip.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "project/project.h"
#include "routine/routinecollection.h"
#include "routine/routine.h"
#include "sequence/stateclip.h"

namespace photon {


class TimelineClipLayer::Impl
{
public:
    Impl(TimelineClipLayer *t_facade);
    SequenceClip *findClip(Clip *, QVector<SequenceClip*>::const_iterator* t_it= nullptr);
    void removeClip(Clip *);
    SequenceClip * addClip(Clip *);
    QVector<SequenceClip*> clips;
    TimelineClipLayer *facade;
};

TimelineClipLayer::Impl::Impl(TimelineClipLayer *t_facade):facade(t_facade)
{

}

SequenceClip *TimelineClipLayer::Impl::findClip(Clip *t_clip, QVector<SequenceClip*>::const_iterator* t_it)
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

SequenceClip *TimelineClipLayer::Impl::addClip(Clip *t_clip)
{
    SequenceClip *sequenceClip = new SequenceClip(t_clip);
    clips.append(sequenceClip);
    facade->scene()->addItem(sequenceClip);
    sequenceClip->setParentItem(facade);
    sequenceClip->setPos(t_clip->startTime(),0);
    return sequenceClip;
}

void TimelineClipLayer::Impl::removeClip(Clip *t_clip)
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


TimelineClipLayer::TimelineClipLayer(ClipLayer *t_layer): LayerItem(t_layer),m_impl(new Impl(this))
{
    connect(t_layer, &ClipLayer::clipAdded, this, &TimelineClipLayer::clipAdded);
    connect(t_layer, &ClipLayer::clipRemoved, this, &TimelineClipLayer::clipRemoved);
    connect(t_layer, &ClipLayer::clipModified, this, &TimelineClipLayer::clipModified);
}

TimelineClipLayer::~TimelineClipLayer()
{
    delete m_impl;
}

SequenceClip *TimelineClipLayer::itemForClip(Clip *t_clip) const
{
    return m_impl->findClip(t_clip);
}

void TimelineClipLayer::addedToScene(TimelineScene *t_scene)
{
    for(Clip *clip : static_cast<ClipLayer*>(layer())->clips())
    {
        m_impl->addClip(clip);
    }
}

void TimelineClipLayer::addClip(SequenceClip *t_clip)
{
    t_clip->setParentItem(this);
    t_clip->setPos(t_clip->clip()->startTime(),0);
}

void TimelineClipLayer::clipAdded(photon::Clip *t_clip)
{
    m_impl->addClip(t_clip);
}

void TimelineClipLayer::clipRemoved(photon::Clip *t_clip)
{
    m_impl->removeClip(t_clip);
}

void TimelineClipLayer::clipModified(photon::Clip *t_clip)
{
    auto sequenceClip = m_impl->findClip(t_clip);
    if(sequenceClip)
    {
        sequenceClip->setPos(t_clip->startTime(),0);
        sequenceClip->updateDuration();
    }
}

void TimelineClipLayer::removeLayer()
{
    layer()->sequence()->removeLayer(layer());
}

QRectF TimelineClipLayer::boundingRect() const
{
    /*
    if(scene())
        return QRectF(0,0,scene()->width(), m_impl->layer->height());

    else
    */
        return QRectF(0,0,300, layer()->height());

}

void TimelineClipLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    painter->fillRect(boundingRect(), QColor(40,40,40));
}

void TimelineClipLayer::addRoutine(photon::Routine *t_routine, double t_time)
{
    RoutineClip *clip = new RoutineClip(t_time, 5.0);
    clip->setRoutine(t_routine);
    static_cast<ClipLayer*>(layer())->addClip(clip);
}

void TimelineClipLayer::addState(double t_time)
{
    StateClip *clip = new StateClip(t_time, 5.0);
    auto state = new State;
    state->addDefaultCapabilities();
    clip->setState(state);
    static_cast<ClipLayer*>(layer())->addClip(clip);
}

void TimelineClipLayer::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
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
    menu.addAction("Add State",[this, time](){addState(time);});

    menu.addSeparator();
    QAction *removeLayer = menu.addAction("Remove Layer");
    connect(removeLayer, &QAction::triggered, this, &TimelineClipLayer::removeLayer);

    menu.exec(event->screenPos());
}

} // namespace photon
