#include <QPainter>
#include "portitem.h"

namespace keira {

class PortItem::Impl
{
public:
    Parameter *parameter;
    PortDirection direction;
    int radius = 6;
    bool isHovering = false;
};

PortItem::PortItem(Parameter *t_param, PortDirection t_direction) : QGraphicsItem(),m_impl(new Impl)
{
    m_impl->parameter = t_param;
    m_impl->direction = t_direction;
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

PortItem::~PortItem()
{
    delete m_impl;
}

void PortItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_impl->isHovering = true;
    update();
}

void PortItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_impl->isHovering = false;
    update();
}

Parameter *PortItem::parameter() const
{
    return m_impl->parameter;
}

PortDirection PortItem::direction() const
{
    return m_impl->direction;
}

QRectF PortItem::boundingRect() const
{
    return QRectF(-m_impl->radius, -m_impl->radius, m_impl->radius * 2.0, m_impl->radius * 2.0);
}

void PortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(m_impl->isHovering ? Qt::cyan : Qt::red);
    painter->drawEllipse(QPoint(0,0), m_impl->radius, m_impl->radius);
}

} // namespace keira
