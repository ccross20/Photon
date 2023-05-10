#ifndef PHOTON_SEQUENCECLIP_H
#define PHOTON_SEQUENCECLIP_H

#include <QGraphicsItem>
#include "photon-global.h"

namespace photon {

class SequenceClip : public QGraphicsItem
{
public:

    enum HitResult
    {
        HitNone,
        HitCenter,
        HitResizeStart,
        HitResizeEnd,
        HitTransitionInEnd,
        HitTransitionInStart,
        HitTransitionOutStart,
        HitTransitionOutEnd
    };



    SequenceClip(Clip *);
    ~SequenceClip();

    void updateDuration();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

    Clip *clip() const;

    HitResult hitTest(const QPointF &, double scale) const;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SEQUENCECLIP_H
