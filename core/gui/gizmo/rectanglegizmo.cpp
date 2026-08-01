
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "rectanglegizmo.h"

namespace photon {

RectangleGizmo::RectangleGizmo(const QRectF &t_rect, std::function<void(QPointF)> t_callback,
                               PositionMode t_mode):QGraphicsItem(),m_callback(t_callback),m_rect(t_rect),m_mode(t_mode)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);
    updateCursor();
}

void RectangleGizmo::setRect(const QRectF &t_rect)
{
    prepareGeometryChange();
    m_rect = t_rect;
}

void RectangleGizmo::setOrientation(Qt::Orientation t_orientation)
{
    m_orientation = t_orientation;
    updateCursor();
}

void RectangleGizmo::updateCursor()
{
    // Cursor advertises which way the handle moves.
    const bool horizontal = m_orientation & Qt::Horizontal;
    const bool vertical   = m_orientation & Qt::Vertical;
    if(horizontal && vertical)
        setCursor(Qt::SizeAllCursor);
    else if(horizontal)
        setCursor(Qt::SizeHorCursor);
    else
        setCursor(Qt::SizeVerCursor);
}

QRectF RectangleGizmo::boundingRect() const
{
    return m_rect;
}

QPainterPath RectangleGizmo::shape() const
{
    QPainterPath path;
    path.addRect(m_rect);
    return path;
}

void RectangleGizmo::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);

    if(m_mode == PositionRelative)
        m_callback(pos());
    else
        m_callback(event->scenePos());
}

void RectangleGizmo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{

    QColor fillColor = Qt::cyan;
    if (option->state & QStyle::State_MouseOver)
        fillColor = Qt::black;


    painter->fillRect(m_rect, fillColor);
}

QVariant RectangleGizmo::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{

    if (change == ItemPositionChange && scene()) {

        QPointF newPos = value.toPointF();
        // Lock to the handle's axis: a horizontal-only handle can't move in y, a
        // vertical-only handle can't move in x. (Default is free 2D movement.)
        if(!(m_orientation & Qt::Horizontal))
            newPos.setX(pos().x());
        if(!(m_orientation & Qt::Vertical))
            newPos.setY(pos().y());
        return newPos;
    }

    return QGraphicsItem::itemChange(change, value);
}




} // namespace photon
