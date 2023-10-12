#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include "timelineviewer.h"
#include "sequenceclip.h"
#include "sequence/clip.h"
#include "timelinescene.h"
#include "layeritem.h"
#include "sequence/cliplayer.h"
#include "sequence/sequence.h"

namespace photon {

class ClipMoveData
{
public:
    ClipMoveData(Clip *t_clip):startTime(t_clip->startTime()),
        startDuration(t_clip->duration()),
        startEaseInDuration(t_clip->easeInDuration()),
        startEaseOutDuration(t_clip->easeOutDuration()),
        startStrength(t_clip->strength()),
        clip(t_clip){}
    double startTime;
    double startDuration;
    double startEaseInDuration;
    double startEaseOutDuration;
    double startStrength;
    Clip *clip;
};


class TimelineViewer::Impl
{
public:
    enum InteractionMode
    {
        InteractionSelect,
        InteractionMove,
        InteractionResizeStart,
        InteractionResizeEnd,
        InteractionResizeEaseIn,
        InteractionResizeEaseOut
    };


    QVector<ClipMoveData> moveDatas;
    QPointF startPoint;
    QPoint lastPosition;
    double scale = 5.0;
    double playheadTime = 0.0;
    InteractionMode interactionMode = InteractionSelect;

};

TimelineViewer::TimelineViewer() : QGraphicsView(),m_impl(new Impl)
{
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //setCacheMode(QGraphicsView::CacheNone);

    setTransform(QTransform::fromScale(m_impl->scale, 1.0));
}

TimelineViewer::~TimelineViewer()
{
    delete m_impl;
}

void TimelineViewer::setScale(double t_value)
{
    if(m_impl->scale == t_value)
        return;
    m_impl->scale = t_value;
    setTransform(QTransform::fromScale(m_impl->scale, 1.0));
    emit scaleChanged(m_impl->scale);
}

void TimelineViewer::movePlayheadTo(double t_time)
{
    double newTime = t_time;

    QRect updateRect;
    if(newTime < m_impl->playheadTime)
        updateRect = QRect(newTime,0,m_impl->playheadTime - newTime,height());
    else
        updateRect = QRect(m_impl->playheadTime,0,newTime - m_impl->playheadTime,height());

    m_impl->playheadTime = newTime;


    //double mappedTime = mapFromScene(QPointF(m_impl->playheadTime,0)).x();

    scene()->update(updateRect.adjusted(-2,0,4,0));
    //scene()->update();
}

void TimelineViewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    painter->fillRect(rect, QColor(25,25,25));


}

void TimelineViewer::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    /*
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::green, mapToScene(QPoint(3,0)).x()));
    painter->drawLine(QLineF{m_impl->playheadTime,0,m_impl->playheadTime,static_cast<double>(height())});
    */

    painter->fillRect(QRectF(m_impl->playheadTime,0,1.0/m_impl->scale,height()), QColor(0,255,0));


}

void TimelineViewer::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

}

void TimelineViewer::mousePressEvent(QMouseEvent *event)
{
    m_impl->startPoint = event->pos();
    m_impl->lastPosition = event->pos();


    auto item = itemAt(event->pos());

    auto clipItem = dynamic_cast<SequenceClip*>(item);

    if(clipItem)
    {
        clipItem->setZValue(100);
        m_impl->moveDatas.append(clipItem->clip());

        auto hitResult = clipItem->hitTest(clipItem->mapFromScene(mapToScene(event->pos())), m_impl->scale);

        switch(hitResult)
        {
            default:
            case SequenceClip::HitNone:
            case SequenceClip::HitCenter:
                m_impl->interactionMode = Impl::InteractionMove;
                break;
            case SequenceClip::HitResizeStart:
                m_impl->interactionMode = Impl::InteractionResizeStart;
                break;
            case SequenceClip::HitResizeEnd:
                m_impl->interactionMode = Impl::InteractionResizeEnd;
                break;
            case SequenceClip::HitTransitionInEnd:
                m_impl->interactionMode = Impl::InteractionResizeEaseIn;
                break;
            case SequenceClip::HitTransitionOutStart:
                m_impl->interactionMode = Impl::InteractionResizeEaseOut;
                break;
        }
    }
    if(!(event->buttons() & Qt::MiddleButton))
        QGraphicsView::mousePressEvent(event);
}

void TimelineViewer::mouseMoveEvent(QMouseEvent *event)
{

    if((event->buttons() & Qt::LeftButton))
    {

        if(event->modifiers() & Qt::ControlModifier)
        {
            double const ZOOM_INCREMENT = 1.1;
            QPointF delta = event->pos() - m_impl->lastPosition;
            if(delta.x() < 0)
                m_impl->scale /= ZOOM_INCREMENT;
            else
                m_impl->scale *= ZOOM_INCREMENT;

            if(m_impl->scale < .001)
                m_impl->scale = .25;


            setTransform(QTransform::fromScale(m_impl->scale, 1.0));

            emit scaleChanged(m_impl->scale);
        }
        else
        {
            auto scenePos = mapToScene(event->pos());
            auto timelineScene = static_cast<TimelineScene*>(scene());
            float time = scenePos.x();
            timelineScene->sequence()->snapToBeat(scenePos.x(),&time,2);
            scenePos.setX(time);

            QPointF delta = scenePos - mapToScene(m_impl->startPoint.toPoint());
            for(const auto &data : m_impl->moveDatas)
            {

                auto layer = timelineScene->layerAtY(mapToScene(event->pos()).y());
                auto clipItem = timelineScene->itemForClip(data.clip);

                if(m_impl->interactionMode == Impl::InteractionMove)
                {



                    if(layer && layer->layer() != data.clip->layer())
                    {
                        auto clipLayer = dynamic_cast<ClipLayer*>(layer->layer());
                        if(clipLayer)
                            clipLayer->addClip(data.clip);
                    }


                    data.clip->setStartTime(data.startTime + delta.x());
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeStart)
                {
                    data.clip->setStartTime(data.startTime + delta.x());
                    data.clip->setDuration(data.startDuration - delta.x());
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeEnd)
                {
                    data.clip->setDuration(data.startDuration + delta.x());
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeEaseIn && clipItem)
                {
                    data.clip->setEaseInDuration(data.startEaseInDuration + delta.x());
                    data.clip->setStrength(data.startStrength - delta.y()/clipItem->boundingRect().height());
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeEaseOut && clipItem)
                {
                    data.clip->setEaseOutDuration(data.startEaseOutDuration - delta.x());
                    data.clip->setStrength(data.startStrength - delta.y()/clipItem->boundingRect().height());
                }
            }
        }
    }
    if((event->buttons() & Qt::MiddleButton))
    {
        QPoint delta = event->pos() - m_impl->lastPosition;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
    }
    else
    {
        auto item = itemAt(event->pos());

        auto clipItem = dynamic_cast<SequenceClip*>(item);

        if(clipItem)
        {
            auto hitResult = clipItem->hitTest(clipItem->mapFromScene(mapToScene(event->pos())), m_impl->scale);

            switch(hitResult)
            {
                default:
                case SequenceClip::HitNone:
                case SequenceClip::HitCenter:
                    setCursor(Qt::ArrowCursor);
                    break;

                case SequenceClip::HitResizeStart:
                case SequenceClip::HitResizeEnd:
                    setCursor(Qt::SizeHorCursor);
                    break;
                case SequenceClip::HitTransitionInEnd:
                case SequenceClip::HitTransitionOutStart:
                    setCursor(Qt::SizeAllCursor);
                    break;
            }
        }
        else
            setCursor(Qt::ArrowCursor);
    }

    m_impl->lastPosition = event->pos();

    if(!(event->buttons() & Qt::MiddleButton))
        QGraphicsView::mouseMoveEvent(event);
}

void TimelineViewer::mouseReleaseEvent(QMouseEvent *event)
{
    for(const auto &data : m_impl->moveDatas)
    {
        auto timelineScene = static_cast<TimelineScene*>(scene());
        auto clipItem = timelineScene->itemForClip(data.clip);
        if(clipItem)
            clipItem->setZValue(0);
    }
    m_impl->moveDatas.clear();
    m_impl->interactionMode = Impl::InteractionSelect;

    if(!(event->buttons() & Qt::MiddleButton))
        QGraphicsView::mouseReleaseEvent(event);
}

void TimelineViewer::wheelEvent(QWheelEvent *event)
{
    QGraphicsView::wheelEvent(event);

    if(event->modifiers() & Qt::ControlModifier)
    {
        m_impl->scale += event->angleDelta().y() / 10.0;

        if(m_impl->scale < .001)
            m_impl->scale = .25;

        setTransform(QTransform::fromScale(m_impl->scale, 1.0));

        emit scaleChanged(m_impl->scale);
    }
}

} // namespace photon
