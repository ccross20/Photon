#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QScrollBar>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include "timelineviewer.h"
#include "sequenceclip.h"
#include "sequence/clip.h"
#include "timelinescene.h"
#include "layeritem.h"
#include "sequence/cliplayer.h"
#include "sequence/sequence.h"

namespace photon {

namespace {
    // A clip can never shrink to zero/negative duration - that's a nonsensical
    // state nothing downstream guards against. Dragging a resize handle past the
    // clip's opposite edge (an easy, fast mouse motion) would otherwise drive
    // duration negative with no floor.
    constexpr double kMinClipDuration = 0.01;

    // How close (in screen pixels) a dragged clip edge must come to another
    // clip's edge or a cue marker before it snaps.
    constexpr double kSnapPixels = 8.0;
}

class ClipMoveData
{
public:
    ClipMoveData(Clip *t_clip, int t_startLayerIndex):startTime(t_clip->startTime()),
        startDuration(t_clip->duration()),
        startEaseInDuration(t_clip->easeInDuration()),
        startEaseOutDuration(t_clip->easeOutDuration()),
        startStrength(t_clip->strength()),
        startLayerIndex(t_startLayerIndex),
        clip(t_clip){}
    double startTime;
    double startDuration;
    double startEaseInDuration;
    double startEaseOutDuration;
    double startStrength;
    int startLayerIndex;   // in Sequence::layers(), for moving across layers together
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
    double startXPos = 0.0;
    double xOffset = 0.0;
    InteractionMode interactionMode = InteractionSelect;
    // Command-drag on a clip: copy the selection once the drag gets going,
    // then move the copies.
    bool duplicatePending = false;

    // Nothing moves until the pointer passes the drag threshold, so a click
    // never nudges a clip.
    bool dragged = false;
    int anchorIndex = 0;   // the pressed clip's entry in moveDatas

    // What a click on an already-selected clip does if it never becomes a drag.
    enum ClickAction { ClickNone, ClickSelectOnly, ClickDeselect };
    ClickAction clickAction = ClickNone;
    Clip *clickedClip = nullptr;

};

TimelineViewer::TimelineViewer() : QGraphicsView(),m_impl(new Impl)
{
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::RubberBandDrag);
    // Needs keyboard focus for the clip clipboard shortcuts.
    setFocusPolicy(Qt::StrongFocus);

    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //setCacheMode(QGraphicsView::CacheNone);
    setSceneRect(QRectF{-5000,0,40000,500});

    QTransform xform;
    xform.translate(-m_impl->xOffset,0);
    xform.scale(m_impl->scale,1.0);

    setTransform(xform);
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

    if(t_value < 2.0)
        m_impl->scale = 2.0;

    QTransform xform;
    xform.translate(-m_impl->xOffset,0);
    xform.scale(m_impl->scale,1.0);


    setTransform(xform);
    emit scaleChanged(m_impl->scale);
}

void TimelineViewer::setOffset(double t_value)
{
    if(m_impl->xOffset == t_value)
        return;
    m_impl->xOffset = t_value;


    QTransform xform;
    xform.translate(-m_impl->xOffset,0);
    xform.scale(m_impl->scale,1.0);


    setTransform(xform);
    emit offsetChanged(m_impl->xOffset);
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
    m_impl->startXPos = (m_impl->startPoint.x() + m_impl->xOffset) / m_impl->scale;
    m_impl->dragged = false;
    m_impl->clickAction = Impl::ClickNone;
    m_impl->clickedClip = nullptr;
    m_impl->duplicatePending = false;

    // Only the left button selects. A right press is left alone: Qt's
    // default would treat it as a click on empty space (clips ignore it) and
    // clear the selection before the context menu - which acts on that
    // selection - even opens. The menu arrives separately via contextMenuEvent.
    if(event->button() != Qt::LeftButton)
        return;

    setFocus();

    // Clicking anywhere in a lane - a clip or empty space - makes that lane
    // the active layer, where pasted clips go.
    auto timelineScene = static_cast<TimelineScene*>(scene());
    if(auto *lane = timelineScene->layerAtY(mapToScene(event->pos()).y()))
        if(auto *clipLayer = dynamic_cast<ClipLayer*>(lane->layer()))
            timelineScene->setActiveLayer(clipLayer);

    // Option-drag zooms (see mouseMoveEvent) - no selecting or rubber band.
    if(event->modifiers() & Qt::AltModifier)
        return;

    auto clipItem = dynamic_cast<SequenceClip*>(itemAt(event->pos()));

    if(!clipItem)
    {
        // Empty space: rubber-band select. Qt only adds to the selection with
        // Control (Command on macOS), so Shift is passed on as that too.
        if(event->modifiers() & Qt::ShiftModifier)
        {
            QMouseEvent additive(event->type(), event->position(), event->scenePosition(), event->globalPosition(),
                                 event->button(), event->buttons(),
                                 (event->modifiers() & ~Qt::ShiftModifier) | Qt::ControlModifier,
                                 event->pointingDevice());
            QGraphicsView::mousePressEvent(&additive);
        }
        else
            QGraphicsView::mousePressEvent(event);
        return;
    }

    // Clip selection is handled here rather than by the scene, which would
    // drop the rest of a multi-selection before it could be dragged.
    const bool shift = event->modifiers() & Qt::ShiftModifier;
    const bool command = event->modifiers() & Qt::ControlModifier;
    m_impl->clickedClip = clipItem->clip();

    if(shift)
    {
        clipItem->setSelected(!clipItem->isSelected());
        if(!clipItem->isSelected())
            return;   // just removed from the selection - nothing to drag
    }
    else if(command)
    {
        if(clipItem->isSelected())
            m_impl->clickAction = Impl::ClickDeselect;
        else
            clipItem->setSelected(true);
    }
    else if(clipItem->isSelected())
        m_impl->clickAction = Impl::ClickSelectOnly;
    else
    {
        scene()->clearSelection();
        clipItem->setSelected(true);
    }

    switch(clipItem->hitTest(clipItem->mapFromScene(mapToScene(event->pos())), m_impl->scale))
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

    // Moving takes the whole selection along; resizing and easing stay
    // per-clip.
    const auto &layers = timelineScene->sequence()->layers();
    auto addMoveData = [this, &layers](SequenceClip *item) {
        item->setZValue(100);
        m_impl->moveDatas.append(ClipMoveData(item->clip(), layers.indexOf(item->clip()->layer())));
    };

    addMoveData(clipItem);
    m_impl->anchorIndex = 0;
    if(m_impl->interactionMode == Impl::InteractionMove)
    {
        for(auto *selected : scene()->selectedItems())
        {
            auto *other = dynamic_cast<SequenceClip*>(selected);
            if(other && other != clipItem)
                addMoveData(other);
        }
    }

    m_impl->duplicatePending = command && m_impl->interactionMode == Impl::InteractionMove;
}

void TimelineViewer::duplicateMovingClips()
{
    m_impl->duplicatePending = false;
    auto timelineScene = static_cast<TimelineScene*>(scene());
    scene()->clearSelection();

    // The originals stay put; the copies take their place in the drag. A copy
    // starts with the original's timing, so the recorded start values still apply.
    for(auto &data : m_impl->moveDatas)
    {
        auto *layer = dynamic_cast<ClipLayer*>(data.clip->layer());
        if(!layer)
            continue;
        Clip *copy = layer->duplicateClip(data.clip);
        if(!copy)
            continue;

        if(auto *originalItem = timelineScene->itemForClip(data.clip))
            originalItem->setZValue(0);
        data.clip = copy;
        if(auto *copyItem = timelineScene->itemForClip(copy))
        {
            copyItem->setZValue(100);
            copyItem->setSelected(true);
        }
    }
}

void TimelineViewer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint deltaPt = event->pos() - m_impl->lastPosition;

    if((event->buttons() & Qt::LeftButton))
    {

        // Option-drag zooms (Command-drag duplicates clips).
        if(event->modifiers() & Qt::AltModifier)
        {
            double newScaleX = m_impl->scale;



            if(deltaPt.x() > 0)
                newScaleX *= 1.1;
            else if(deltaPt.x() < 0)
                newScaleX /= 1.1;


            setScale(newScaleX);

            double newXPos = (m_impl->startPoint.x() + m_impl->xOffset) / newScaleX;

            setOffset(m_impl->xOffset - ((newXPos - m_impl->startXPos) * newScaleX) );
            m_impl->startXPos = (m_impl->startPoint.x() + m_impl->xOffset) / newScaleX;

            /*
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
*/
        }
        else
        {
            if(!m_impl->moveDatas.isEmpty() && !m_impl->dragged)
            {
                // Hold off until it's clearly a drag, so a click doesn't nudge
                // clips and a Command-click doesn't leave a stacked copy behind.
                if((event->pos() - m_impl->startPoint.toPoint()).manhattanLength() < QApplication::startDragDistance())
                {
                    m_impl->lastPosition = event->pos();
                    return;
                }
                m_impl->dragged = true;
                if(m_impl->duplicatePending)
                    duplicateMovingClips();
            }

            auto scenePos = mapToScene(event->pos());
            auto timelineScene = static_cast<TimelineScene*>(scene());
            QPointF delta = scenePos - mapToScene(m_impl->startPoint.toPoint());

            // Snap the clips' own edges (not the pointer, which could be
            // anywhere inside a clip) to other clips' edges and cue markers.
            // Whichever edge needs the smallest correction wins, and the
            // tolerance is in screen pixels so it feels the same at any zoom.
            if(!m_impl->moveDatas.isEmpty())
            {
                QVector<Clip*> excludeClips;
                for(const auto &data : m_impl->moveDatas)
                    excludeClips.append(data.clip);

                const float tolerance = float(kSnapPixels / m_impl->scale);
                double bestAdjust = 0.0;
                bool snapped = false;
                auto snapEdge = [&](double edge) {
                    float snappedTime = 0.0f;
                    if(!timelineScene->sequence()->snapTime(float(edge), &snappedTime, tolerance, excludeClips))
                        return;
                    const double adjust = snappedTime - edge;
                    if(!snapped || std::abs(adjust) < std::abs(bestAdjust))
                    {
                        bestAdjust = adjust;
                        snapped = true;
                    }
                };

                const ClipMoveData &anchor = m_impl->moveDatas[m_impl->anchorIndex];
                switch(m_impl->interactionMode)
                {
                    case Impl::InteractionMove:
                        for(const auto &data : m_impl->moveDatas)
                        {
                            snapEdge(data.startTime + delta.x());
                            snapEdge(data.startTime + data.startDuration + delta.x());
                        }
                        break;
                    case Impl::InteractionResizeStart:
                        snapEdge(anchor.startTime + delta.x());
                        break;
                    case Impl::InteractionResizeEnd:
                        snapEdge(anchor.startTime + anchor.startDuration + delta.x());
                        break;
                    default:
                        break;   // easing handles don't snap
                }
                delta.setX(delta.x() + bestAdjust);
            }

            // Layer changes follow the pressed clip: every moving clip shifts
            // by the same number of layers, and only if all of them land on a
            // clip layer - otherwise they all stay where they are.
            const auto &layers = timelineScene->sequence()->layers();
            int layerShift = 0;
            bool canChangeLayers = false;
            if(m_impl->interactionMode == Impl::InteractionMove && !m_impl->moveDatas.isEmpty())
            {
                const auto *layerUnderCursor = timelineScene->layerAtY(mapToScene(event->pos()).y());
                const int target = layerUnderCursor ? layers.indexOf(layerUnderCursor->layer()) : -1;
                const int anchorStart = m_impl->moveDatas[m_impl->anchorIndex].startLayerIndex;
                if(target >= 0 && anchorStart >= 0)
                {
                    layerShift = target - anchorStart;
                    canChangeLayers = true;
                    for(const auto &data : m_impl->moveDatas)
                    {
                        const int index = data.startLayerIndex + layerShift;
                        if(data.startLayerIndex < 0 || index < 0 || index >= layers.size()
                           || !dynamic_cast<ClipLayer*>(layers[index]))
                        {
                            canChangeLayers = false;
                            break;
                        }
                    }
                }
            }

            for(const auto &data : m_impl->moveDatas)
            {
                auto clipItem = timelineScene->itemForClip(data.clip);

                if(m_impl->interactionMode == Impl::InteractionMove)
                {
                    if(canChangeLayers)
                    {
                        auto *destination = static_cast<ClipLayer*>(layers[data.startLayerIndex + layerShift]);
                        if(destination != data.clip->layer())
                        {
                            // Changing layer replaces the clip's timeline item
                            // with a fresh one, so carry the selection and
                            // drag stacking over to it.
                            destination->addClip(data.clip);
                            if(auto *newItem = timelineScene->itemForClip(data.clip))
                            {
                                newItem->setSelected(true);
                                newItem->setZValue(100);
                            }
                        }
                    }
                    data.clip->setStartTime(data.startTime + delta.x());
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeStart)
                {
                    data.clip->setStartTime(data.startTime + delta.x());
                    data.clip->setDuration(std::max(kMinClipDuration, data.startDuration - delta.x()));
                }
                else if(m_impl->interactionMode == Impl::InteractionResizeEnd)
                {
                    data.clip->setDuration(std::max(kMinClipDuration, data.startDuration + delta.x()));
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
        setOffset(m_impl->xOffset - delta.x());
       // horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
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
    // A click (no drag) on an already-selected clip: plain click narrows the
    // selection to it, Command-click removes it.
    if(!m_impl->dragged && m_impl->clickedClip && m_impl->clickAction != Impl::ClickNone)
    {
        if(auto *item = static_cast<TimelineScene*>(scene())->itemForClip(m_impl->clickedClip))
        {
            if(m_impl->clickAction == Impl::ClickSelectOnly)
            {
                scene()->clearSelection();
                item->setSelected(true);
            }
            else
                item->setSelected(false);
        }
    }

    m_impl->moveDatas.clear();
    m_impl->interactionMode = Impl::InteractionSelect;
    m_impl->duplicatePending = false;
    m_impl->clickAction = Impl::ClickNone;
    m_impl->clickedClip = nullptr;

    if(!(event->buttons() & Qt::MiddleButton))
        QGraphicsView::mouseReleaseEvent(event);
}

void TimelineViewer::keyPressEvent(QKeyEvent *event)
{
    auto timelineScene = static_cast<TimelineScene*>(scene());
    if(event->matches(QKeySequence::Copy))
        timelineScene->copySelectedClips();
    else if(event->matches(QKeySequence::Cut))
        timelineScene->cutSelectedClips();
    else if(event->matches(QKeySequence::Paste))
        timelineScene->pasteClips(m_impl->playheadTime);   // into the active layer
    else
        QGraphicsView::keyPressEvent(event);
}

void TimelineViewer::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier)
    {
        // Zoom around the cursor while preserving the pan. Route through
        // setScale/setOffset so the offset-preserving transform is applied and the
        // host keeps the other views in sync. (The old path used fromScale(), which
        // dropped the offset and desynced the views.)
        const double cursorX = event->position().x();
        const double cursorTime = (cursorX + m_impl->xOffset) / m_impl->scale;

        // Scaled by how far this one event actually scrolled rather than a flat
        // per-event step - a high-resolution device (Magic Mouse, trackpad)
        // fires many small events per gesture, and a flat step compounds across
        // all of them into a runaway zoom. ZOOM_PER_NOTCH is deliberately
        // gentle (was 1.1, a flat 10% every event); see WaveformWidget's
        // matching fix, since the two views stay zoom-synced.
        constexpr double kZoomPerNotch = 1.04;
        const QPoint pixels = event->pixelDelta();
        const QPoint angle = event->angleDelta();
        const double notches = !pixels.isNull()
            ? (pixels.y() != 0 ? pixels.y() : pixels.x()) / 40.0
            : (angle.y() != 0 ? angle.y() : angle.x()) / 120.0;
        const double factor = std::pow(kZoomPerNotch, notches);
        setScale(m_impl->scale * factor);
        setOffset(cursorTime * m_impl->scale - cursorX);
        event->accept();
    }
    else
    {
        // Horizontal pan. Route through setOffset so xOffset updates and
        // offsetChanged fires, keeping the waveform and channel views in sync. The
        // default QGraphicsView wheel handler scrolls this view's own scroll area,
        // which moves only this view and desyncs the others.
        const QPoint pixels = event->pixelDelta();
        const QPoint angle = event->angleDelta();

        double delta;
        if(!pixels.isNull())
            delta = pixels.y() != 0 ? pixels.y() : pixels.x();
        else
        {
            const int a = angle.y() != 0 ? angle.y() : angle.x();
            delta = (a / 120.0) * 40.0;   // ~40px per wheel notch
        }

        setOffset(m_impl->xOffset - delta);
        event->accept();
    }
}

} // namespace photon
