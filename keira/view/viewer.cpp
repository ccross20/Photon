#include <QMouseEvent>
#include <QScrollBar>
#include <QJsonObject>
#include <QUuid>
#include "viewer.h"
#include "library/nodelibrary.h"
#include "nodeitem.h"
#include "portitem.h"
#include "wireitem.h"
#include "model/node.h"
#include "scene.h"
#include "model/graph.h"
#include "model/parameter/parameter.h"
#include "model/subgraphnode.h"

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
    NodeLibrary *library = nullptr;
    double currentZoom = 1.0;

    // Ctrl was pressed on this node and the mouse hasn't moved far enough yet
    // to say whether it's a clone-drag or just a Ctrl+click on the selection.
    NodeItem *clonePendingItem = nullptr;
    QPointF clonePressScenePos;

    // The copy currently being dragged, once that decision has been made.
    NodeItem *cloneDragItem = nullptr;
    QPointF cloneGrabOffset;
};

Viewer::Viewer(NodeLibrary *t_library, QWidget *parent) : QGraphicsView{parent},m_impl(new Impl)
{
    m_impl->library = t_library;
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

NodeItem *Viewer::startCloneDrag(NodeItem *t_source, const QPointF &t_scenePos)
{
    if(!t_source || !m_impl->library || !graph())
        return nullptr;

    Node *sourceNode = t_source->node();
    if(!sourceNode)
        return nullptr;

    Node *copy = m_impl->library->createNode(sourceNode->id());
    if(!copy)
        return nullptr;   // node type isn't registered in this library

    QJsonObject json;
    sourceNode->writeToJson(json);
    // A node's uniqueId round-trips through its json, so reading it straight
    // back would leave two nodes claiming the same id - which findNode() and
    // saved connections both key on. Give the copy a fresh one instead.
    json.insert("uniqueId", QString(QUuid::createUuid().toByteArray(QUuid::WithoutBraces)));
    copy->readFromJson(json, m_impl->library);

    // Connections live on the Graph rather than in a node's own json, so the
    // copy comes out unwired - which is what's wanted here.
    graph()->addNode(copy);
    // addNode() only queues; drain so the Scene has actually built the item to
    // hand the drag to. Forcing a drain from this thread is the same thing
    // Sequence::save() does, and the queue's own mutexes cover the eval thread.
    graph()->drainCommandQueue();

    NodeItem *copyItem = static_cast<Scene*>(scene())->itemForNode(copy);
    if(!copyItem)
        return nullptr;

    scene()->clearSelection();
    copyItem->setSelected(true);

    m_impl->cloneDragItem = copyItem;
    m_impl->cloneGrabOffset = copyItem->pos() - t_scenePos;
    return copyItem;
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    m_impl->startPoint = event->scenePosition();
    m_impl->lastPosition = event->pos();

    if(event->buttons() & Qt::MiddleButton || m_impl->key == Qt::Key_Space)
    {
        setDragMode(QGraphicsView::NoDrag);
        return;
    }

    // Starting on a port or wire begins a wire drag (handled below), so keep the
    // view passive there. Anywhere else, enable the rubber-band so a drag over
    // empty canvas draws a selection rectangle; pressing a movable node still
    // moves it (Qt prefers item drags over the rubber-band).
    auto pressItem = itemAt(event->pos());
    if(dynamic_cast<PortItem*>(pressItem) || dynamic_cast<WireItem*>(pressItem))
        setDragMode(QGraphicsView::NoDrag);
    else
        setDragMode(QGraphicsView::RubberBandDrag);

    // Ctrl on a node's own body arms a clone-drag. Whether it becomes one is
    // decided on the first move, so a plain Ctrl+click still toggles selection
    // the way it always did. The press is deliberately not forwarded: letting
    // the base class see it would start Qt dragging the ORIGINAL node, and the
    // whole point is that the original stays where it is.
    //
    // Testing for the NodeItem itself (rather than any descendant) keeps this
    // off the parameter widgets, where Ctrl+drag already means something - a
    // coarse scrub in NumberScrubField.
    if(event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier))
    {
        if(auto *nodeItem = dynamic_cast<NodeItem*>(pressItem))
        {
            setDragMode(QGraphicsView::NoDrag);
            m_impl->clonePendingItem = nodeItem;
            m_impl->clonePressScenePos = mapToScene(event->pos());
            event->accept();
            return;
        }
    }

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

    auto itemsUnderCursor = items(event->pos());
    for(auto item : itemsUnderCursor)
    {
        auto nodeItem = dynamic_cast<NodeItem*>(item);
        if(nodeItem)
        {
            if(nodeItem->node()->isContainer())
            {
                emit subGraphClicked(static_cast<SubGraphNode*>(nodeItem->node())->graph());

                return;
            }

        }
    }
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

    if(m_impl->cloneDragItem)
    {
        // setPos goes through NodeItem::itemChange, which writes the position
        // back to the node and repaints its wires - the same path Qt's own item
        // dragging uses.
        m_impl->cloneDragItem->setPos(mapToScene(event->pos()) + m_impl->cloneGrabOffset);
        event->accept();
        return;
    }

    if(m_impl->clonePendingItem)
    {
        const QPointF scenePos = mapToScene(event->pos());
        // Same few-pixel threshold the wire drag uses, so a shaky Ctrl+click
        // doesn't leave a stray copy behind.
        if((scenePos - m_impl->clonePressScenePos).manhattanLength() < 3.0)
            return;

        NodeItem *pending = m_impl->clonePendingItem;
        m_impl->clonePendingItem = nullptr;
        if(startCloneDrag(pending, m_impl->clonePressScenePos))
            m_impl->cloneDragItem->setPos(scenePos + m_impl->cloneGrabOffset);
        event->accept();
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
                    if(nodeItem->node()->isContainer())
                    {

                    }
                    else
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
        }

        m_impl->draggingWire->setPointB(mapToScene(event->pos()));


    }
    QGraphicsView::mouseMoveEvent(event);
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{

    if(event->buttons() & Qt::MiddleButton || m_impl->key == Qt::Key_Space)
        return;

    if(m_impl->cloneDragItem)
    {
        m_impl->cloneDragItem = nullptr;
        event->accept();
        return;
    }

    if(m_impl->clonePendingItem)
    {
        // Ctrl went down on a node but the mouse never moved, so this was a
        // Ctrl+click after all - apply the add-to-selection the base class
        // would have done if the press had been forwarded.
        m_impl->clonePendingItem->setSelected(!m_impl->clonePendingItem->isSelected());
        m_impl->clonePendingItem = nullptr;
        event->accept();
        return;
    }

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
                // A plain click (press+release with no drag in between) on a port
                // still goes through this whole start-a-wire/drop-a-wire path, and
                // without moving the mouse the port under the cursor at release is
                // the very port the drag started from - connecting it to itself.
                // Just cancel the drag in that case instead.
                if(portItem != m_impl->startPort)
                {
                    if(portItem->direction() == Input)
                        graph()->connectParameters(m_impl->startPort->parameter(), portItem->parameter());
                    else
                        graph()->connectParameters(portItem->parameter(), m_impl->startPort->parameter());
                }
                QGraphicsView::mouseReleaseEvent(event);
                return;
            }
            else
            {
                auto nodeItem = dynamic_cast<NodeItem*>(item);
                if(nodeItem)
                {
                    if(nodeItem->node()->isContainer())
                    {
                        Parameter *mirror = nullptr;
                        auto *subGraph = dynamic_cast<SubGraphNode*>(nodeItem->node());

                        // Dropping an OUTPUT wire onto a subgraph creates the matching
                        // input node inside the graph and connects to its auto-mirrored
                        // outer port — so a parameter can be exposed from either side.
                        if(subGraph && m_impl->startPort->direction() == Output)
                            mirror = subGraph->exposeInputForType(m_impl->startPort->parameter()->typeId());

                        if(mirror)
                        {
                            graph()->connectParameters(m_impl->startPort->parameter(), mirror);
                        }
                        else
                        {
                            // Fallback (unsupported type, or an input-direction drop):
                            // clone the parameter straight onto the node as before.
                            auto param = m_impl->startPort->parameter()->clone(m_impl->library);
                            param->setId(QUuid::createUuid().toByteArray(QUuid::WithoutBraces));
                            param->setConnectionOptions(keira::AllowSingleInput);
                            param->setName(m_impl->startPort->parameter()->node()->name() + ":" + param->name());
                            nodeItem->node()->addParameter(param);
                            nodeItem->addPort();

                            if(m_impl->startPort->direction() == Output)
                                graph()->connectParameters(m_impl->startPort->parameter(), param);
                            else
                                graph()->connectParameters(param, m_impl->startPort->parameter());
                        }

                        QGraphicsView::mouseReleaseEvent(event);
                    }
                    else
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
    // Delete can be pressed mid-drag, and the item being dragged is the one
    // most likely to be selected - drop the references before they dangle.
    m_impl->clonePendingItem = nullptr;
    m_impl->cloneDragItem = nullptr;

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
            zoom(m_impl->currentZoom + (event->angleDelta().y()/240.0), pt);
        #endif

        return;
    }

    QGraphicsView::wheelEvent(event);
}


} // namespace keira
