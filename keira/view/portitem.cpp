#include <QPainter>
#include <QHash>
#include "portitem.h"
#include "model/parameter/parameter.h"

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

QColor PortItem::colorForType(const QByteArray &t_typeId)
{
    // A Tableau-20-derived categorical palette: picked for mutual
    // distinctness rather than any thematic meaning, so adjacent types in
    // the node editor are easy to tell apart at a glance.
    static const QHash<QByteArray, QColor> colors{
        {"any",          QColor(0x79, 0x70, 0x6e)},
        {"boolean",      QColor(0xe1, 0x57, 0x59)},
        {"button",       QColor(0xff, 0x9d, 0xa7)},
        {"integer",      QColor(0x59, 0xa1, 0x4f)},
        {"decimal",      QColor(0x4e, 0x79, 0xa7)},
        {"string",       QColor(0xed, 0xc9, 0x48)},
        {"option",       QColor(0xb0, 0x7a, 0xa1)},
        {"stringOption", QColor(0xd4, 0xa6, 0xc8)},
        {"color",        QColor(0xd3, 0x72, 0x95)},
        {"colorPalette", QColor(0xfa, 0xbf, 0xd2)},
        {"vector3D",     QColor(0x76, 0xb7, 0xb2)},
        {"point2D",      QColor(0x86, 0xbc, 0xb6)},
        {"path",         QColor(0xb6, 0x99, 0x2d)},
        {"texture",      QColor(0x9c, 0x75, 0x5f)},
        {"canvas",       QColor(0x9d, 0x76, 0x60)},
        {"pixelList",    QColor(0xf2, 0x8e, 0x2b)},
        {"dmxMatrix",    QColor(0x49, 0x98, 0x94)},
        {"fixture",      QColor(0xf1, 0xce, 0x63)},
        {"fixtureList",  QColor(0x6a, 0x4c, 0x93)},
    };
    return colors.value(t_typeId, colors.value("any"));
}

void PortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    const QColor base = colorForType(m_impl->parameter ? m_impl->parameter->typeId() : QByteArray("any"));
    painter->setBrush(m_impl->isHovering ? base.lighter(150) : base);
    painter->drawEllipse(QPoint(0,0), m_impl->radius, m_impl->radius);
}

} // namespace keira
