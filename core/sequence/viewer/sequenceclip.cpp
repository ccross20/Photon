#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include "sequenceclip.h"
#include "sequence/clip.h"
#include "sequence/layer.h"

namespace photon {

class SequenceClip::Impl
{
public:
    Impl(SequenceClip *t_facade, Clip *t_clip);
    Clip *clip;
    SequenceClip *facade;
    bool isHovering = false;
};

SequenceClip::Impl::Impl(SequenceClip *t_facade, Clip *t_clip):clip(t_clip),facade(t_facade)
{

}

SequenceClip::SequenceClip(Clip *t_clip) : QGraphicsItem(),m_impl(new Impl(this, t_clip))
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);

}

SequenceClip::~SequenceClip()
{
    delete m_impl;
}

void SequenceClip::updateDuration()
{
    prepareGeometryChange();
    update();
}

QRectF SequenceClip::boundingRect() const
{
    return QRectF(0,0,m_impl->clip->duration(), m_impl->clip->layer()->height());
}

void SequenceClip::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    double xScale = painter->transform().m11();

    painter->scale(1.0/xScale,1.0);

    QRectF scaledRect = boundingRect();
    scaledRect.setWidth(scaledRect.width() * xScale);

    painter->fillRect(scaledRect, m_impl->isHovering ? Qt::yellow : Qt::red);
    if(isSelected())
    {
        painter->setPen(QPen(Qt::cyan,2));
        painter->drawRect(scaledRect.adjusted(1,1,-1,-1));
    }
    painter->setClipRect(scaledRect);
    painter->drawText(2,20,"Clip");
}

Clip *SequenceClip::clip() const
{
    return m_impl->clip;
}

SequenceClip::HitResult SequenceClip::hitTest(const QPointF &t_point, double t_scale) const
{

    if(t_point.x() <  5/t_scale)
        return HitResizeStart;
    if(t_point.x() > boundingRect().width() - 5/t_scale)
        return HitResizeEnd;

    return HitCenter;
}

void SequenceClip::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_impl->isHovering = true;
    update();
}

void SequenceClip::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{

}

void SequenceClip::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_impl->isHovering = false;
    update();
}

void SequenceClip::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsItem::contextMenuEvent(event);
    if(event->isAccepted())
        return;

    event->accept();
    Clip *clip = m_impl->clip;

    QMenu menu;
    menu.addAction("Remove Clip", [clip](){
        clip->layer()->removeClip(clip);
        delete clip;
    });

    menu.exec(event->screenPos());
}

} // namespace photon
