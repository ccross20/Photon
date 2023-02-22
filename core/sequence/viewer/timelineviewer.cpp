#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include "timelineviewer.h"
#include "sequenceclip.h"
#include "sequence/clip.h"

namespace photon {

class ClipMoveData
{
public:
    ClipMoveData(SequenceClip *t_seqClip):startTime(t_seqClip->clip()->startTime()),startDuration(t_seqClip->clip()->duration()),sequenceClip(t_seqClip),clip(t_seqClip->clip()){}
    double startTime;
    double startDuration;
    SequenceClip *sequenceClip;
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
        InteractionResizeEnd
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
        m_impl->moveDatas.append(clipItem);

        auto hitResult = clipItem->hitTest(clipItem->mapFromScene(mapToScene(event->pos())), m_impl->scale);

        switch(hitResult)
        {
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

            QPointF delta = mapToScene(event->pos()) - mapToScene(m_impl->startPoint.toPoint());
            for(const auto &data : m_impl->moveDatas)
            {
                if(m_impl->interactionMode == Impl::InteractionMove)
                    data.clip->setStartTime(data.startTime + delta.x());
                else if(m_impl->interactionMode == Impl::InteractionResizeStart)
                {
                    data.clip->setStartTime(data.startTime + delta.x());
                    data.clip->setDuration(data.startDuration - delta.x());
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeEnd)
                {
                    data.clip->setDuration(data.startDuration + delta.x());
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
                case SequenceClip::HitNone:
                case SequenceClip::HitCenter:
                    setCursor(Qt::ArrowCursor);
                    break;

                case SequenceClip::HitResizeStart:
                case SequenceClip::HitResizeEnd:
                    setCursor(Qt::SizeHorCursor);
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
        data.sequenceClip->setZValue(0);
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
