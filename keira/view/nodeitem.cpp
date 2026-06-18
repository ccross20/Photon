#include <QPainter>
#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QGraphicsLayoutItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QLineEdit>
#include "nodeitem.h"
#include "model/node.h"
#include "model/parameter/parameter.h"
#include "wireitem.h"
#include "portitem.h"

namespace keira {

class NodeItemParameter
{
public:
    Parameter *param;
    PortItem *inputPort = nullptr;
    PortItem *outputPort = nullptr;
};

class NodeItemPort
{
public:
    NodeItemPort(){}
    NodeItemPort(Parameter *, PortDirection, QPointF);
    Parameter *param;
    PortDirection direction;
    PortItem *item = nullptr;
    QPointF centerPt;
};

class NodeItem::Impl
{
public:
    Impl(NodeItem *t_item);
    void layoutNode();
    bool parameterExists(Parameter *);

    NodeItem *facade;
    Node *node;
    QVector<NodeItemParameter> parameters;
    QVector<WireItem*> wires;
    int height = 20;
    int width = 120;
    int inset = 2;
    int paramTextInset = 10;
};

NodeItemPort::NodeItemPort(Parameter *t_param, PortDirection t_direction, QPointF t_pt):param(t_param),
    direction(t_direction),
    centerPt(t_pt)
{

}

NodeItem::Impl::Impl(NodeItem *t_item):facade(t_item)
{

}

bool NodeItem::Impl::parameterExists(Parameter *t_param)
{
    for(auto checkParam : parameters)
    {
        if(checkParam.param == t_param)
        {
            return true;
        }
    }
    return false;
}

void NodeItem::Impl::layoutNode()
{
    height = 20;
    for(Parameter *param : node->parameters())
    {
        if(!param->allowInput() && !param->allowOutput())
            continue;
        height += param->rowHeight();
    }
    height += 5;

    facade->setGeometry(QRectF{node->position().x(), node->position().y(),static_cast<double>(width), static_cast<double>(height)});

}

NodeItem::NodeItem(Node *t_node) : QGraphicsWidget(),m_impl(new Impl(this))
{
    m_impl->node = t_node;
    m_impl->width = t_node->width();
    updatePosition();
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);

    m_impl->layoutNode();

}

NodeItem::~NodeItem()
{
    delete m_impl;
}

PortItem *NodeItem::snapToPort(QPointF t_position, PortDirection t_direction)
{
    double y = 20;
    for(const auto &param : m_impl->parameters)
    {
        if(!param.param->allowInput() && !param.param->allowOutput())
            continue;

        int rowHeight = param.param->rowHeight();

        if(t_position.y() >= y && t_position.y() <  y + rowHeight)
            return t_direction == Input ? param.inputPort : param.outputPort;

        y += rowHeight;
    }
    return nullptr;
}

QVariant NodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionHasChanged)
    {
        m_impl->node->setPosition(value.toPointF());
        for(WireItem *wire : m_impl->wires)
            wire->updatePoints();
    }
    return value;
}

void NodeItem::addedToScene(QGraphicsScene *t_scene)
{
    double y = 20;
    for(Parameter *param : m_impl->node->parameters())
    {
        if(!param->allowInput() && !param->allowOutput())
            continue;
        NodeItemParameter itemParam;
        itemParam.param = param;
        int rowHeight = param->rowHeight();

        int options = param->layoutOptions();

        if(options & Parameter::LayoutNoLabel)
        {

        }


        if(param->allowInput())
        {
            itemParam.inputPort = new PortItem(param, Input);
            t_scene->addItem(itemParam.inputPort);
            itemParam.inputPort->setParentItem(this);
            itemParam.inputPort->setPos(QPointF(m_impl->inset, y + rowHeight / 2));
        }

        if(param->allowOutput())
        {
            itemParam.outputPort = new PortItem(param, Output);
            t_scene->addItem(itemParam.outputPort);
            itemParam.outputPort->setParentItem(this);
            itemParam.outputPort->setPos(QPointF(m_impl->width - m_impl->inset, y + rowHeight / 2));
        }

        m_impl->parameters.append(itemParam);
        y += rowHeight;
    }

}

void NodeItem::addPort()
{


    prepareGeometryChange();
    double y = 20;
    for(Parameter *param : m_impl->node->parameters())
    {
        if(!param->allowInput() && !param->allowOutput())
            continue;

        NodeItemParameter itemParam;
        itemParam.param = param;
        int rowHeight = param->rowHeight();

        int options = param->layoutOptions();

        if(options & Parameter::LayoutNoLabel)
        {

        }

        if(!m_impl->parameterExists(param))
        {
            if(param->allowInput())
            {
                itemParam.inputPort = new PortItem(param, Input);
                scene()->addItem(itemParam.inputPort);
                itemParam.inputPort->setParentItem(this);
                itemParam.inputPort->setPos(QPointF(m_impl->inset, y + rowHeight / 2));
            }

            if(param->allowOutput())
            {
                itemParam.outputPort = new PortItem(param, Output);
                scene()->addItem(itemParam.outputPort);
                itemParam.outputPort->setParentItem(this);
                itemParam.outputPort->setPos(QPointF(m_impl->width - m_impl->inset, y + rowHeight / 2));
            }

            m_impl->parameters.append(itemParam);
        }

        y += rowHeight;
    }

    m_impl->layoutNode();
    updateGeometry();
}

void NodeItem::updatePosition()
{
    setPos(m_impl->node->position());
    for(WireItem *wire : m_impl->wires)
        wire->updatePoints();
}

void NodeItem::updateFromNode()
{
}

Node *NodeItem::node() const
{
    return m_impl->node;
}

void NodeItem::widgetUpdated(QWidget *t_widget, const keira::Parameter *t_param)
{
    m_impl->node->setValue(t_param->id(), t_param->updateValue(t_widget));
}
/*
bool NodeItem::contains(const QPointF &point) const
{
    return boundingRect().contains(point);
}
*/
PortItem *NodeItem::port(Parameter *t_param, PortDirection t_direction) const
{
    for(const auto &param : m_impl->parameters)
    {
        if(param.param == t_param)
        {
            if(t_direction == Input)
                return param.inputPort;
            else
                return param.outputPort;
        }
    }
    return nullptr;
}

void NodeItem::addWire(WireItem *t_wire)
{
    m_impl->wires.append(t_wire);
}

void NodeItem::removeWire(WireItem *t_wire)
{
    m_impl->wires.removeOne(t_wire);
}

QRectF NodeItem::boundingRect() const
{
    return QRect{0,0,m_impl->width, m_impl->height};
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(isSelected() ? Qt::cyan : Qt::gray);
    painter->drawRoundedRect(boundingRect().adjusted(m_impl->inset,m_impl->inset,-m_impl->inset,-m_impl->inset),8,8, Qt::SizeMode::RelativeSize);

    int counter = 0;
    int y = 20;
    for(Parameter *param : m_impl->node->parameters())
    {
        if(!param->allowInput() && !param->allowOutput())
            continue;
        QRect rect(m_impl->inset, y, m_impl->width - (2 * m_impl->inset), param->rowHeight());
        if(counter %2 == 0)
        {
            painter->fillRect(rect, QColor(255,255,255,100));
        }
        y += param->rowHeight();
        ++counter;
    }

    auto f = painter->font();
    f.setBold(true);
    painter->setPen(Qt::black);
    painter->setFont(f);
    painter->drawText(QRect(5,5,150,30), m_impl->node->name());


    f.setBold(false);
    painter->setFont(f);

    counter = 0;
    y = 20;
    for(Parameter *param : m_impl->node->parameters())
    {
        if(!param->allowInput() && !param->allowOutput())
            continue;
        QRect rect(m_impl->paramTextInset, y, m_impl->width - (2 * m_impl->paramTextInset), param->rowHeight());

        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, param->name());
        y += param->rowHeight();
        ++counter;
    }


}


} // namespace keira
