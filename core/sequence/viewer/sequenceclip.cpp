#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include "sequenceclip.h"
#include "sequence/clip.h"
#include "sequence/cliplayer.h"

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
    painter->setRenderHint(QPainter::Antialiasing);
    double xScale = painter->transform().m11();

    painter->scale(1.0/xScale,1.0);

    QRectF scaledRect = boundingRect();
    scaledRect.setWidth(scaledRect.width() * xScale);

    float inX = m_impl->clip->easeInDuration() * xScale;
    float outX = (boundingRect().width() - m_impl->clip->easeOutDuration()) * xScale;
    float strengthY = m_impl->clip->layer()->height() * m_impl->clip->strength();
    float h = m_impl->clip->layer()->height();

    painter->fillRect(scaledRect.adjusted(1,1,-1,-1), QColor(Qt::red).darker());
    QPainterPath path;
    path.moveTo(0,scaledRect.height());


    if(m_impl->clip->easeInDuration() > 0 && m_impl->clip->easeInDuration() * xScale > 2)
    {

        double w = m_impl->clip->easeInDuration() * xScale;

        QEasingCurve inCurve(m_impl->clip->easeInType());
        double t = 0.0;
        double step = 1.0 / w;
        for(int i = 1; i < w; ++i)
        {
            path.lineTo(i,h - (inCurve.valueForProgress(t) * strengthY));
            t+= step;
        }
        path.lineTo(inX,h - strengthY);
    }
    else
    {
        path.lineTo(0,h - strengthY);
    }

    if(m_impl->clip->easeOutDuration() > 0 && m_impl->clip->easeOutDuration() * xScale > 1)
    {
        double w = m_impl->clip->easeOutDuration() * xScale;

        QEasingCurve outCurve(m_impl->clip->easeOutType());
        double t = 0.0;
        double step = 1.0 / w;
        for(int i = 0; i < w; ++i)
        {
            path.lineTo(outX + i,h - ( outCurve.valueForProgress(1-t) * strengthY));
            t+= step;
        }
    }
    else
    {
        path.lineTo(scaledRect.right(),h - strengthY);
    }
    path.lineTo(scaledRect.right(),scaledRect.height());
    path.closeSubpath();

    painter->fillPath(path, m_impl->isHovering ? Qt::yellow : Qt::red);
    if(isSelected())
    {
        painter->setPen(QPen(Qt::cyan,2));
        painter->drawRect(scaledRect.adjusted(1,1,-1,-1));
    }
    painter->setClipRect(scaledRect);

    float handleY = (m_impl->clip->layer()->height() - 4) * m_impl->clip->strength();

    painter->fillRect(QRectF(scaledRect.topLeft() + QPointF(inX, h - 4 - handleY),QSize(4,4)), Qt::black);
    painter->fillRect(QRectF(scaledRect.topLeft() + QPointF(outX, h - 4 - handleY) - QPoint(4,0),QSize(4,4)), Qt::black);
    painter->drawText(inX + 2,20,m_impl->clip->name());

}

Clip *SequenceClip::clip() const
{
    return m_impl->clip;
}

SequenceClip::HitResult SequenceClip::hitTest(const QPointF &t_point, double t_scale) const
{
    QPointF localPt(t_point.x() * t_scale, t_point.y());

    float inX = m_impl->clip->easeInDuration();
    float outX = (boundingRect().width() - m_impl->clip->easeOutDuration());
    float strengthY = (m_impl->clip->layer()->height()-4) * (1.0 - m_impl->clip->strength());

    QRectF easeInRect( QPointF(inX * t_scale, strengthY),QSize(4,4));
    QRectF easeOutRect( QPointF(outX * t_scale, strengthY)- QPoint(4,0),QSize(4,4));

    if(easeInRect.contains(localPt))
        return HitTransitionInEnd;
    if(easeOutRect.contains(localPt))
        return HitTransitionOutStart;
    if(localPt.x() <  5)
        return HitResizeStart;
    if(localPt.x() > boundingRect().width() * t_scale - 5)
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

    auto easeInMenu = menu.addMenu("Ease In Mode");
    easeInMenu->addAction("Linear", [clip](){clip->setEaseInType(QEasingCurve::Type::Linear);});
    easeInMenu->addAction("In Out Curve", [clip](){clip->setEaseInType(QEasingCurve::Type::InOutCubic);});
    easeInMenu->addAction("In Curve", [clip](){clip->setEaseInType(QEasingCurve::Type::InCubic);});
    easeInMenu->addAction("Out Curve", [clip](){clip->setEaseInType(QEasingCurve::Type::OutCubic);});

    auto easeOutMenu = menu.addMenu("Ease Out Mode");
    easeOutMenu->addAction("Linear", [clip](){clip->setEaseOutType(QEasingCurve::Type::Linear);});
    easeOutMenu->addAction("In Out Curve", [clip](){clip->setEaseOutType(QEasingCurve::Type::InOutCubic);});
    easeOutMenu->addAction("In Curve", [clip](){clip->setEaseOutType(QEasingCurve::Type::InCubic);});
    easeOutMenu->addAction("Out Curve", [clip](){clip->setEaseOutType(QEasingCurve::Type::OutCubic);});

    menu.exec(event->screenPos());
}

} // namespace photon
