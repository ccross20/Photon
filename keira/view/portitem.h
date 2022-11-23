#ifndef KEIRA_PORTITEM_H
#define KEIRA_PORTITEM_H
#include <QGraphicsItem>
#include "keira-global.h"

namespace keira {

class PortItem : public QGraphicsItem
{
public:
    PortItem(Parameter *, PortDirection);
    ~PortItem();

    Parameter *parameter() const;
    PortDirection direction() const;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_PORTITEM_H
