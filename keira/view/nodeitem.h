#ifndef KEIRA_NODEITEM_H
#define KEIRA_NODEITEM_H
#include "keira-global.h"
#include <QGraphicsWidget>

namespace keira {

class WireItem;
class PortItem;

class KEIRA_EXPORT NodeItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    NodeItem(Node *);
    ~NodeItem();

    Node *node() const;
    //bool contains(const QPointF &point) const override;
    QRectF boundingRect() const override;
    PortItem *port(Parameter *, PortDirection) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    PortItem *snapToPort(QPointF t_position, PortDirection t_direction);
    void updatePosition();
    void updateFromNode();

    void addedToScene(QGraphicsScene *);
    void addWire(WireItem *);
    void removeWire(WireItem *);

public slots:
    void widgetUpdated(QWidget *, const keira::Parameter *);

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_NODEITEM_H
