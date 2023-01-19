#include <QMouseEvent>
#include <QScrollBar>
#include "viewer.h"
#include "nodeitem.h"
#include "portitem.h"
#include "wireitem.h"
#include "model/node.h"
#include "scene.h"
#include "model/graph.h"

namespace keira {

struct NodePositionData
{
    NodePositionData(){}
    NodePositionData(NodeItem *t_item, QPointF t_position):item(t_item),position(t_position){}

    NodeItem *item;
    QPointF position;
};


class Viewer::Impl
{
public:
    QPointF startPoint;
    QPoint lastPosition;
    int key = -1;
    QVector<NodePositionData> items;
    PortItem *startPort = nullptr;
    WireItem *draggingWire = nullptr;
    double currentZoom = 1.0;
};

Viewer::Viewer(QWidget *parent) : QGraphicsView{parent},m_impl(new Impl)
{

    setSceneRect(QRect(-5000,-5000,10000,10000));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

Viewer::~Viewer()
{
    delete m_impl;
}

Graph *Viewer::graph() const
{
    return static_cast<Scene*>(scene())->graph();
}

void Viewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(25,25,25));
}

void Viewer::zoom(double t_value)
{
    if(t_value < .25)
        t_value = .25;
    else if(t_value > 3.25)
        t_value = 3.25;
    m_impl->currentZoom = t_value;
    setTransform(QTransform::fromScale(t_value, t_value));
    emit zoomChanged(m_impl->currentZoom);
}

void Viewer::zoom(double t_value, QPointF pt)
{
    if(t_value < .25)
        t_value = .25;
    else if(t_value > 3.25)
        t_value = 3.25;
    m_impl->currentZoom = t_value;
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setTransform(QTransform::fromScale(t_value, t_value));

    emit zoomChanged(m_impl->currentZoom);
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    m_impl->startPoint = event->scenePosition();
    m_impl->lastPosition = event->pos();

    if(event->buttons() & Qt::MiddleButton || m_impl->key == Qt::Key_Space)
        return;


    QGraphicsView::mousePressEvent(event);

    auto item = itemAt(event->pos());

    m_impl->startPort = dynamic_cast<PortItem*>(item);
    if(m_impl->startPort)
    {
        m_impl->draggingWire = new WireItem(m_impl->startPort->scenePos(), m_impl->startPort->scenePos());

        if(m_impl->startPort->direction() == Input)
            m_impl->draggingWire->setIsReversed(true);
        scene()->addItem(m_impl->draggingWire);
    }

    WireItem *wire = dynamic_cast<WireItem*>(item);
    if(wire)
    {
        QPointF startPt = wire->hoverPercent() > .5 ? wire->pointA() : wire->pointB();
        m_impl->draggingWire = new WireItem(startPt, mapToScene(event->pos()));
        if(wire->hoverPercent() < .5)
        {
            m_impl->draggingWire->setIsReversed(true);
            m_impl->startPort = wire->inPort();
        }
        else
            m_impl->startPort = wire->outPort();
        graph()->disconnectParameters(wire->outParameter(), wire->inParameter());
        scene()->addItem(m_impl->draggingWire);
    }


}

void Viewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsView::mouseDoubleClickEvent(event);
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton || (m_impl->key == Qt::Key_Space && event->buttons() & Qt::LeftButton))
    {
        QPoint delta = m_impl->lastPosition - event->pos();
        m_impl->lastPosition = event->pos();
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        return;
    }

    if(m_impl->draggingWire)
    {
        auto itemsUnderCursor = items(event->pos());
        for(auto item : itemsUnderCursor)
        {
            auto portItem = dynamic_cast<PortItem*>(item);
            if(portItem)
            {
                m_impl->draggingWire->setPointB(portItem->scenePos());
                QGraphicsView::mouseMoveEvent(event);
                return;
            }
            else
            {
                auto nodeItem = dynamic_cast<NodeItem*>(item);
                if(nodeItem)
                {
                    PortItem *port = nodeItem->snapToPort(nodeItem->mapFromScene(mapToScene(event->pos())), m_impl->startPort->direction() == Input ? Output : Input);
                    if(port)
                    {
                        m_impl->draggingWire->setPointB(port->scenePos());
                        QGraphicsView::mouseMoveEvent(event);
                        return;
                    }
                }
            }
        }

        m_impl->draggingWire->setPointB(mapToScene(event->pos()));


    }
    QGraphicsView::mouseMoveEvent(event);
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{

    if(event->buttons() & Qt::MiddleButton || m_impl->key == Qt::Key_Space)
        return;
    if(m_impl->draggingWire)
    {
        //scene()->removeItem(m_impl->draggingWire);
        delete m_impl->draggingWire;
        m_impl->draggingWire = nullptr;

        auto itemsUnderCursor = items(event->pos());
        for(auto item : itemsUnderCursor)
        {
            auto portItem = dynamic_cast<PortItem*>(item);
            if(portItem)
            {
                if(portItem->direction() == Input)
                    graph()->connectParameters(m_impl->startPort->parameter(), portItem->parameter());
                else
                    graph()->connectParameters(portItem->parameter(), m_impl->startPort->parameter());
                QGraphicsView::mouseReleaseEvent(event);
                return;
            }
            else
            {
                auto nodeItem = dynamic_cast<NodeItem*>(item);
                if(nodeItem)
                {
                    PortItem *port = nodeItem->snapToPort(nodeItem->mapFromScene(mapToScene(event->pos())), m_impl->startPort->direction() == Input ? Output : Input);
                    if(port)
                    {
                        if(port->direction() == Input)
                            graph()->connectParameters(m_impl->startPort->parameter(), port->parameter());
                        else
                            graph()->connectParameters(port->parameter(), m_impl->startPort->parameter());
                        QGraphicsView::mouseReleaseEvent(event);
                        return;
                    }
                }
            }
        }



    }
    QGraphicsView::mouseReleaseEvent(event);
}

void Viewer::keyPressEvent(QKeyEvent *event)
{
    m_impl->key = event->key();
    QGraphicsView::keyPressEvent(event);

    if(event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        deleteSelected();
    }
}

void Viewer::keyReleaseEvent(QKeyEvent *event)
{
    m_impl->key = -1;
    QGraphicsView::keyReleaseEvent(event);
}

void Viewer::deleteSelected()
{
    auto selectedItems = scene()->selectedItems();

    for(auto item : selectedItems)
    {
        auto nodeItem = dynamic_cast<NodeItem*>(item);

        if(nodeItem)
        {
            graph()->removeNode(nodeItem->node());
        }
    }
}

void Viewer::wheelEvent(QWheelEvent *event)
{

    if(event->buttons() & Qt::LeftButton)
        return;

    if(event->modifiers() & Qt::ControlModifier)
    {
        QPointF pt = mapToScene(event->globalPosition().toPoint());

        #if defined(Q_OS_WIN)
            zoom(m_impl->currentZoom + (event->angleDelta().y()/420.0), pt);
        #elif defined(Q_OS_MAC)
            zoom(m_currentZoom + (event->angleDelta().y()/240.0), pt);
        #endif

        return;
    }

    QGraphicsView::wheelEvent(event);
}


} // namespace keira
