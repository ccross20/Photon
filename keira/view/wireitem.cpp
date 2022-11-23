#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include "wireitem.h"
#include "nodeitem.h"
#include "portitem.h"

namespace keira {

class WireItem::Impl
{
public:
    void updatePoints();
    void buildPath();

    QPainterPath path;
    QPainterPath hoverStroke;
    QPointF pointA, pointB;
    PortItem *outPort;
    PortItem *inPort;
    QPointF hoverHandlePoint;
    double hoverPercent = 0.0;
    bool isHovering = false;
    bool isReversed = false;
    bool isEmpty = true;
};

void WireItem::Impl::updatePoints()
{
    pointA = outPort->scenePos();
    pointB = inPort->scenePos();
    buildPath();
}

void WireItem::Impl::buildPath()
{
    path = QPainterPath();
    isEmpty = true;

    QPointF ptA = pointA;
    QPointF ptB = pointB;

    if(isReversed)
        std::swap(ptA, ptB);

    path.moveTo(ptA);
    //path.lineTo(pointB);

    double handleOffset = qMin(40.0, abs(ptA.y() - ptB.y()) * .4);

    if(ptA != ptB)
    {
        QPointF startPivot{ptA.x()+handleOffset, ptA.y()};
        QPointF endPivot{ptB.x()-handleOffset, ptB.y()};
        QLineF line{startPivot, endPivot};
        QLineF lineSegment{line.pointAt(handleOffset / line.length()), line.pointAt((line.length() - handleOffset) / line.length())};

        path.quadTo(startPivot, lineSegment.p1());
        path.lineTo(lineSegment.p2());
        path.quadTo(endPivot, ptB);
    }


    isEmpty = false;


    QPainterPathStroker stroker;
    stroker.setWidth(12);
    hoverStroke = stroker.createStroke(path);

}

WireItem::WireItem() : QGraphicsItem(), m_impl(new Impl)
{

}


WireItem::WireItem(PortItem *t_outPort, PortItem *t_inPort):
    QGraphicsItem(), m_impl(new Impl)
{
    m_impl->inPort = t_inPort;
    m_impl->outPort = t_outPort;
    setAcceptHoverEvents(true);

    updatePoints();
}

WireItem::WireItem(const QPointF t_ptA, const QPointF t_ptB):
    QGraphicsItem(), m_impl(new Impl)
{
    m_impl->pointA = t_ptA;
    m_impl->pointB = t_ptB;
    m_impl->buildPath();
}

WireItem::~WireItem()
{
    delete m_impl;
}

void WireItem::destroy()
{
    prepareGeometryChange();
}

QPointF WireItem::pointA() const
{
    return m_impl->pointA;
}

QPointF WireItem::pointB() const
{
    return m_impl->pointB;
}

PortItem *WireItem::inPort() const
{
    return m_impl->inPort;
}

PortItem *WireItem::outPort() const
{
    return m_impl->outPort;
}

Parameter *WireItem::outParameter() const
{
    return m_impl->outPort->parameter();
}

Parameter *WireItem::inParameter() const
{
    return m_impl->inPort->parameter();
}

void WireItem::setIsReversed(bool t_isReversed)
{
    if(m_impl->isReversed == t_isReversed)
        return;
    m_impl->isReversed = t_isReversed;
    m_impl->buildPath();
}

bool WireItem::isReversed() const
{
    return m_impl->isReversed;
}

QRectF WireItem::boundingRect() const
{
    return m_impl->hoverStroke.boundingRect().adjusted(-5,-5,5,5);
}

QPainterPath WireItem::shape() const
{
    return m_impl->hoverStroke;
}

void WireItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QBrush(Qt::red),2.0));
    painter->drawPath(m_impl->path);

    if(m_impl->isHovering)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::red);
        painter->drawEllipse(m_impl->hoverHandlePoint, 5,5);
    }

}

double WireItem::hoverPercent() const
{
    return m_impl->hoverPercent;
}


bool WireItem::contains(const QPointF &point) const
{
    return m_impl->hoverStroke.contains(point);
}

void WireItem::setPointB(const QPointF t_pt)
{
    prepareGeometryChange();
    m_impl->pointB = t_pt;
    m_impl->buildPath();
    update();
}

void WireItem::updatePoints()
{
    prepareGeometryChange();
    m_impl->updatePoints();
    update();
}

double percentAtPointOnCurve(const QPainterPath &path, const QPointF &pt, uint iterations = 8)
{
    double center = .5;
    double length = .5;
    for(uint i = 0; i < iterations; ++i)
    {
        length *= .5;
        QPointF testPtA = path.pointAtPercent(center + length);
        QPointF testPtB = path.pointAtPercent(center - length);

        if(QLineF(testPtA,pt).length() < QLineF(testPtB,pt).length())
        {
            center += length;
        } else {
            center -= length;
        }
    }
    return center;
}

void WireItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_impl->isHovering = true;

    m_impl->hoverPercent = percentAtPointOnCurve(m_impl->path, event->pos());
    m_impl->hoverHandlePoint = m_impl->path.pointAtPercent(m_impl->hoverPercent);
    update();
}

void WireItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    m_impl->isHovering = true;
    m_impl->hoverPercent = percentAtPointOnCurve(m_impl->path, event->pos());
    m_impl->hoverHandlePoint = m_impl->path.pointAtPercent(m_impl->hoverPercent);
    update();
}

void WireItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_impl->isHovering = false;

    update();
}


} // namespace keira
