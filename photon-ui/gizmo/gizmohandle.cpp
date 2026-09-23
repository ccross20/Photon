#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "gizmohandle.h"
#include "gizmogroup.h"

namespace photon {

GizmoHandle::GizmoHandle(GizmoGroup *t_group, Style t_style)
    : QGraphicsItem(), m_group(t_group), m_style(t_style)
{
    setFlags(QGraphicsItem::ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);
    setCursor(Qt::SizeAllCursor);
}

void GizmoHandle::setDataGetter(std::function<QPointF()> t_getter)
{
    m_getter = std::move(t_getter);
    refreshFromData();
}

void GizmoHandle::setDataSetter(std::function<void(QPointF)> t_setter)
{
    m_setter = std::move(t_setter);
    // Only editable handles are movable; read-only handles just display.
    setFlag(QGraphicsItem::ItemIsMovable, bool(m_setter));
}

void GizmoHandle::setConstraint(Qt::Orientations t_constraint)
{
    m_constraint = t_constraint;
    if(m_constraint == (Qt::Horizontal | Qt::Vertical))
        setCursor(Qt::SizeAllCursor);
    else if(m_constraint & Qt::Horizontal)
        setCursor(Qt::SizeHorCursor);
    else
        setCursor(Qt::SizeVerCursor);
}

void GizmoHandle::setSize(qreal t_size)
{
    prepareGeometryChange();
    m_size = t_size;
}

void GizmoHandle::setColor(const QColor &t_color)
{
    m_color = t_color;
    update();
}

QPointF GizmoHandle::dataPosition() const
{
    return m_getter ? m_getter() : QPointF();
}

void GizmoHandle::refreshFromData()
{
    if(!m_getter)
        return;
    m_applying = true;
    setPos(m_group->transform().map(m_getter()));
    m_applying = false;
}

QRectF GizmoHandle::boundingRect() const
{
    const qreal h = m_size / 2.0;
    return QRectF(-h, -h, m_size, m_size);
}

QPainterPath GizmoHandle::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void GizmoHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QColor fill = m_color;
    if(option->state & QStyle::State_MouseOver)
        fill = Qt::black;

    const QRectF r = boundingRect();
    if(m_style == Tangent)
    {
        // Diamond, to read as a control point distinct from the square anchors.
        QPolygonF diamond;
        diamond << QPointF(r.center().x(), r.top())
                << QPointF(r.right(), r.center().y())
                << QPointF(r.center().x(), r.bottom())
                << QPointF(r.left(), r.center().y());
        painter->setPen(Qt::NoPen);
        painter->setBrush(fill);
        painter->drawPolygon(diamond);
    }
    else
    {
        painter->fillRect(r, fill);
    }
}

QVariant GizmoHandle::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange && scene() && !m_applying)
    {
        // 'value' is the proposed new scene (screen) position. Convert it back to
        // data space, apply the axis constraint there, hand it to the effect, then
        // snap to wherever the effect actually landed (it may clamp the value).
        const QTransform &t = m_group->transform();
        QPointF data = t.inverted().map(value.toPointF());
        const QPointF current = dataPosition();

        if(!(m_constraint & Qt::Horizontal))
            data.setX(current.x());
        if(!(m_constraint & Qt::Vertical))
            data.setY(current.y());

        if(m_setter)
        {
            m_setter(data);
            data = dataPosition();   // re-read: honours any clamping the setter did
        }

        m_group->handleMoved(this);
        return t.map(data);
    }

    return QGraphicsItem::itemChange(change, value);
}

void GizmoHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_group->setActiveHandle(this);
    QGraphicsItem::mousePressEvent(event);
}

void GizmoHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    m_group->setActiveHandle(nullptr);
    refreshFromData();
}

} // namespace photon
