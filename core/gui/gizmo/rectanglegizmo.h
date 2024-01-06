#ifndef PHOTON_RECTANGLEGIZMO_H
#define PHOTON_RECTANGLEGIZMO_H

#include <QGraphicsItem>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT RectangleGizmo : public QGraphicsItem
{
public:
    enum PositionMode{
        PositionRelative,
        PositionAbsolute
    };


    RectangleGizmo(const QRectF &rect, std::function<void(QPointF)>, PositionMode mode = PositionRelative);

    void setRect(const QRectF &);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void setOrientation(Qt::Orientation);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int m_orientation = Qt::Horizontal | Qt::Vertical;
    std::function<void(QPointF)> m_callback;
    QRectF m_rect;
    PositionMode m_mode;
};

} // namespace photon

#endif // PHOTON_RECTANGLEGIZMO_H
