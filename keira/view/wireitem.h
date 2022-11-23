#ifndef KEIRA_WIREITEM_H
#define KEIRA_WIREITEM_H

#include <QGraphicsItem>
#include "keira-global.h"

namespace keira {

class PortItem;

class WireItem : public QGraphicsItem
{
public:
    enum WireStyle {
        Linear,
        Curved
    };

    WireItem();
    WireItem(const QPointF, const QPointF);
    WireItem(PortItem *t_outPort, PortItem *t_inPort);
    ~WireItem();

    void destroy();
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    bool contains(const QPointF &point) const override;

    Parameter *outParameter() const;
    Parameter *inParameter() const;
    PortItem *inPort() const;
    PortItem *outPort() const;
    QPointF pointA() const;
    QPointF pointB() const;

    void setIsReversed(bool);
    bool isReversed() const;
    void updatePoints();
    void setPointB(const QPointF);
    double hoverPercent() const;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_WIREITEM_H
