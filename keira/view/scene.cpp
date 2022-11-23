#include <QGraphicsLineItem>
#include <QTimer>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "scene.h"
#include "model/graph.h"
#include "view/nodeitem.h"
#include "model/node.h"
#include "model/graphevaluator.h"
#include "model/parameter/parameter.h"
#include "wireitem.h"
#include "library/nodelibrary.h"

namespace keira {

class Scene::Impl
{
public:
    QHash<Node*,NodeItem*> nodeMap;
    QVector<WireItem*> wires;
    NodeLibrary *library = nullptr;
    Graph *graph = nullptr;
    GraphEvaluator *evaluator;
    QGraphicsLineItem *wireParentItem;
};


Scene::Scene(QObject *parent)
    : QGraphicsScene{parent},m_impl(new Impl)
{
    m_impl->evaluator = new GraphEvaluator(nullptr,this);

    connect(m_impl->evaluator, &GraphEvaluator::evaluationComplete, this, &Scene::updateFromNodes);

    m_impl->wireParentItem = new QGraphicsLineItem;
    addItem(m_impl->wireParentItem);
}

Scene::~Scene()
{
    delete m_impl;
}

void Scene::setIsAutoEvaluate(bool t_value)
{
    m_impl->evaluator->setIsEnabled(t_value);
}

bool Scene::isAutoEvaluate() const
{
    return m_impl->evaluator->isEnabled();
}

void Scene::setNodeLibrary(NodeLibrary *t_library)
{
    m_impl->library = t_library;
}

void Scene::setGraph(Graph *t_graph)
{
    if(m_impl->graph == t_graph)
        return;
    clear();

    m_impl->wireParentItem = new QGraphicsLineItem;
    addItem(m_impl->wireParentItem);

    if(m_impl->graph)
    {
        disconnect(m_impl->graph, &Graph::nodeWasAdded, this, &Scene::nodeWasAdded);
        disconnect(m_impl->graph, &Graph::nodeWasRemoved, this, &Scene::nodeWasRemoved);
        disconnect(m_impl->graph, &Graph::nodePositionUpdated, this, &Scene::nodePositionUpdated);
        disconnect(m_impl->graph, &Graph::parametersWereConnected, this, &Scene::parametersWereConnected);
        disconnect(m_impl->graph, &Graph::parametersWereDisconnected, this, &Scene::parametersWereDisconnected);
    }

    m_impl->graph = t_graph;
    m_impl->evaluator->setGraph(m_impl->graph);

    if(!m_impl->graph)
        return;

    for(auto node : t_graph->nodes())
    {
        nodeWasAdded(node);
    }

    for(auto node : t_graph->nodes())
    {
        for(Parameter *param : node->parameters())
        {
            for(Parameter *inputParam : param->outputParameters())
            {
                parametersWereConnected(param, inputParam);
            }
        }
    }


    connect(m_impl->graph, &Graph::nodeWasAdded, this, &Scene::nodeWasAdded);
    connect(m_impl->graph, &Graph::nodeWasRemoved, this, &Scene::nodeWasRemoved);
    connect(m_impl->graph, &Graph::nodePositionUpdated, this, &Scene::nodePositionUpdated);
    connect(m_impl->graph, &Graph::parametersWereConnected, this, &Scene::parametersWereConnected);
    connect(m_impl->graph, &Graph::parametersWereDisconnected, this, &Scene::parametersWereDisconnected);
}

Graph *Scene::graph() const
{
    return m_impl->graph;
}

void Scene::updateFromNodes()
{
    for(auto it = m_impl->nodeMap.begin(); it != m_impl->nodeMap.end(); ++it)
        (*it)->updateFromNode();
}

NodeItem *Scene::itemForNode(Node *t_node) const
{
    return m_impl->nodeMap.value(t_node, nullptr);
}

void Scene::nodeWasAdded(keira::Node *t_node)
{
    NodeItem *item = new NodeItem{t_node};

    m_impl->nodeMap.insert(t_node, item);

    addItem(item);
    item->addedToScene(this);
}

void Scene::nodeWasRemoved(keira::Node *t_node)
{
    NodeItem *item = itemForNode(t_node);
    if(!item)
        return;

    removeItem(item);
}

void Scene::nodePositionUpdated(keira::Node *t_node)
{

    NodeItem *item = itemForNode(t_node);
    if(!item)
        return;
    item->updatePosition();
}

void Scene::parametersWereConnected(keira::Parameter *t_out, keira::Parameter *t_in)
{
    NodeItem *outItem = itemForNode(t_out->node());
    NodeItem *inItem = itemForNode(t_in->node());

    if(!outItem || !inItem)
    {
        qWarning() << "Items could not be found";
        return;
    }

    PortItem *outPort = outItem->port(t_out, Output);
    PortItem *inPort = inItem->port(t_in, Input);
    WireItem *wire = new WireItem(outPort, inPort);
    addItem(wire);
    outItem->addWire(wire);
    inItem->addWire(wire);
    m_impl->wires.append(wire);

    wire->setParentItem(m_impl->wireParentItem);
}

void Scene::parametersWereDisconnected(keira::Parameter *t_out, keira::Parameter *t_in)
{
    NodeItem *outItem = itemForNode(t_out->node());
    NodeItem *inItem = itemForNode(t_in->node());

    if(!outItem || !inItem)
    {
        qWarning() << "Items could not be found";
        return;
    }

    for(auto wire : m_impl->wires)
    {
        if(wire->outParameter() == t_out && wire->inParameter() == t_in)
        {
            outItem->removeWire(wire);
            inItem->removeWire(wire);
            //wire->destroy();
            //removeItem(wire);
            m_impl->wires.removeOne(wire);
            delete wire;
            return;
        }
    }

}

void Scene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    QGraphicsScene::contextMenuEvent(contextMenuEvent);

    if(contextMenuEvent->isAccepted())
        return;

    QMenu menu;

    QMenu *rootMenu = menu.addMenu("Add Node");
    auto treeRoot = m_impl->library->createNodeTree([](const keira::NodeInformation &info){return true;});

    std::function<void(QMenu &menu, keira::NodeTreeElement *)> treeLoop;
    treeLoop = [&treeLoop](QMenu &menu, keira::NodeTreeElement *rootElement){
        for(uint i = 0; i < rootElement->childCount(); ++i)
        {
            auto element = rootElement->elementAt(i);

            if(element->isFolder())
            {

                treeLoop(*menu.addMenu(element->name()), element);
            }
            else
            {
                auto action = menu.addAction(element->name());
                action->setData(static_cast<keira::NodeElement*>(element)->info().nodeId);
            }
        }
    };

    treeLoop(*rootMenu, treeRoot);

    QAction *action = menu.exec(contextMenuEvent->screenPos());

    if(!action)
        return;

    if(!action->data().isNull())
    {
        Node *node = m_impl->library->createNode(action->data().toByteArray());
        node->setPosition(contextMenuEvent->scenePos());
        m_impl->graph->addNode(node);
    }
}


} // namespace keira
