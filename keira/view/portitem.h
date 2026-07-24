#ifndef KEIRA_PORTITEM_H
#define KEIRA_PORTITEM_H
#include <QGraphicsItem>
#include <QColor>
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

    // One consistent color per parameter typeId, shared with WireItem so a
    // connection reads as the same color as the ports it joins. Unknown
    // types fall back to the same neutral gray as the generic "any" type.
    static QColor colorForType(const QByteArray &typeId);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_PORTITEM_H
