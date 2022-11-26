
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "rectanglegizmo.h"

namespace photon {

RectangleGizmo::RectangleGizmo(const QRectF &t_rect, std::function<void(QPointF)> t_callback):QGraphicsItem(),m_callback(t_callback),m_rect(t_rect)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);
}

void RectangleGizmo::setRect(const QRectF &t_rect)
{
    prepareGeometryChange();
    m_rect = t_rect;
}

void RectangleGizmo::setOrientation(Qt::Orientation t_orientation)
{
    m_orientation = t_orientation;
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

    m_callback(pos());
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
        /*
        if(!(m_orientation & Qt::Vertical))
            newPos.setY(pos().y());
        if(!(m_orientation == Qt::Horizontal))
            newPos.setX(pos().x());
            */
        return newPos;
    }

    return QGraphicsItem::itemChange(change, value);
}




} // namespace photon
