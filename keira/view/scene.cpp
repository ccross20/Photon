#include <QApplication>
#include <QClipboard>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QUuid>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneDragDropEvent>
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
    ExternalDropInterpreter externalDropInterpreter;
};


Scene::Scene(QObject *parent)
    : QGraphicsScene{parent},m_impl(new Impl)
{
    m_impl->evaluator = new GraphEvaluator(nullptr, {}, {}, this);

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

void Scene::setExternalDropInterpreter(ExternalDropInterpreter t_interpreter)
{
    m_impl->externalDropInterpreter = t_interpreter;
}

void Scene::setGraph(Graph *t_graph)
{
    if(m_impl->graph == t_graph)
        return;

    if(m_impl->graph)
        emit graphAboutToChange(m_impl->graph);

    if(m_impl->graph)
    {
        disconnect(m_impl->graph, &Graph::nodeWasAdded, this, &Scene::nodeWasAdded);
        disconnect(m_impl->graph, &Graph::nodeWasRemoved, this, &Scene::nodeWasRemoved);
        disconnect(m_impl->graph, &Graph::nodePositionUpdated, this, &Scene::nodePositionUpdated);
        disconnect(m_impl->graph, &Graph::parametersWereConnected, this, &Scene::parametersWereConnected);
        disconnect(m_impl->graph, &Graph::parametersWereDisconnected, this, &Scene::parametersWereDisconnected);
        disconnect(m_impl->graph, &Graph::nodePortsChanged, this, &Scene::nodePortsChanged);
    }

    m_impl->graph = t_graph;
    m_impl->evaluator->setGraph(m_impl->graph);

    rebuildScene();

    if(!m_impl->graph)
        return;

    connect(m_impl->graph, &Graph::nodeWasAdded, this, &Scene::nodeWasAdded);
    connect(m_impl->graph, &Graph::nodeWasRemoved, this, &Scene::nodeWasRemoved);
    connect(m_impl->graph, &Graph::nodePositionUpdated, this, &Scene::nodePositionUpdated);
    connect(m_impl->graph, &Graph::parametersWereConnected, this, &Scene::parametersWereConnected);
    connect(m_impl->graph, &Graph::parametersWereDisconnected, this, &Scene::parametersWereDisconnected);
    connect(m_impl->graph, &Graph::nodePortsChanged, this, &Scene::nodePortsChanged);

    emit graphUpdated(m_impl->graph);
}

// Rebuild only the affected node's ports (and the wires touching it), leaving
// the rest of the scene intact — so selection and the node-editor panel are
// preserved when a node exposes/unexposes a parameter at runtime.
void Scene::nodePortsChanged(keira::Node *t_node)
{
    NodeItem *item = itemForNode(t_node);
    if(!item)
        return;

    // Drop wires touching this node; they reference ports about to be recreated.
    QVector<WireItem*> affected;
    for(auto *wire : m_impl->wires)
    {
        if(wire->outParameter()->node() == t_node || wire->inParameter()->node() == t_node)
            affected.append(wire);
    }
    for(auto *wire : affected)
    {
        if(auto *outItem = itemForNode(wire->outParameter()->node()))
            outItem->removeWire(wire);
        if(auto *inItem = itemForNode(wire->inParameter()->node()))
            inItem->removeWire(wire);
        m_impl->wires.removeOne(wire);
        delete wire;
    }

    item->rebuildPorts();

    // Recreate wires for this node's remaining connections.
    for(Parameter *param : t_node->parameters())
    {
        for(Parameter *inParam : param->outputParameters())
            parametersWereConnected(param, inParam);
        for(Parameter *outParam : param->inputParameters())
            parametersWereConnected(outParam, param);
    }
}

// Clear and rebuild all node items and wires from the current graph. Used on
// graph switch and when a node's ports change at runtime (surface graphs are
// small, so a full rebuild is simplest and robust).
void Scene::rebuildScene()
{
    clear();
    m_impl->nodeMap.clear();
    m_impl->wires.clear();

    m_impl->wireParentItem = new QGraphicsLineItem;
    addItem(m_impl->wireParentItem);

    if(!m_impl->graph)
        return;

    for(auto node : m_impl->graph->nodes())
        nodeWasAdded(node);

    for(auto node : m_impl->graph->nodes())
    {
        for(Parameter *param : node->parameters())
        {
            for(Parameter *inputParam : param->outputParameters())
                parametersWereConnected(param, inputParam);
        }
    }
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

const char *Scene::NodeClipboardMime = "application/x-keira-nodes";

bool Scene::hasSelectedNodes() const
{
    for(auto *item : selectedItems())
    {
        auto *nodeItem = dynamic_cast<NodeItem*>(item);
        if(nodeItem && nodeItem->node()->isRemovable())
            return true;
    }
    return false;
}

bool Scene::clipboardHasNodes()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    return mime && mime->hasFormat(NodeClipboardMime);
}

void Scene::copySelectedNodes()
{
    QVector<Node*> nodes;
    for(auto *item : selectedItems())
    {
        auto *nodeItem = dynamic_cast<NodeItem*>(item);
        if(nodeItem && nodeItem->node()->isRemovable())
            nodes.append(nodeItem->node());
    }
    if(nodes.isEmpty())
        return;

    // Any edit still queued (e.g. a node just added) should be in the copy.
    if(m_impl->graph)
        m_impl->graph->drainCommandQueue();

    QJsonArray nodeArray;
    QJsonArray connectionArray;
    for(Node *node : nodes)
    {
        QJsonObject nodeObj;
        node->writeToJson(nodeObj);
        nodeArray.append(nodeObj);

        for(Parameter *param : node->parameters())
        {
            for(Parameter *target : param->outputParameters())
            {
                if(!nodes.contains(target->node()))
                    continue;
                QJsonObject connection;
                connection.insert("outputNode", QString(node->uniqueId()));
                connection.insert("outputParameter", QString(param->id()));
                connection.insert("inputNode", QString(target->node()->uniqueId()));
                connection.insert("inputParameter", QString(target->id()));
                connectionArray.append(connection);
            }
        }
    }

    QJsonObject root;
    root.insert("nodes", nodeArray);
    root.insert("connections", connectionArray);

    auto *mime = new QMimeData;
    mime->setData(NodeClipboardMime, QJsonDocument(root).toJson(QJsonDocument::Compact));
    QApplication::clipboard()->setMimeData(mime);
}

void Scene::cutSelectedNodes()
{
    copySelectedNodes();
    for(auto *item : selectedItems())
    {
        auto *nodeItem = dynamic_cast<NodeItem*>(item);
        if(nodeItem && nodeItem->node()->isRemovable())
            m_impl->graph->removeNode(nodeItem->node());
    }
}

void Scene::pasteNodes(const QPointF &t_scenePos)
{
    if(!m_impl->graph || !m_impl->library || !clipboardHasNodes())
        return;

    const QJsonObject root = QJsonDocument::fromJson(
        QApplication::clipboard()->mimeData()->data(NodeClipboardMime)).object();
    const QJsonArray nodeArray = root.value("nodes").toArray();
    if(nodeArray.isEmpty())
        return;

    // Keep the group's layout, centred on the paste point.
    QPointF centre;
    for(const auto &value : nodeArray)
    {
        const QJsonObject position = value.toObject().value("position").toObject();
        centre += QPointF(position.value("x").toDouble(), position.value("y").toDouble());
    }
    centre /= nodeArray.size();
    const QPointF shift = t_scenePos - centre;

    const QByteArray graphType = m_impl->graph->graphTypeId();
    QHash<QString, Node*> pastedById;   // copied node's uniqueId -> its paste
    for(const auto &value : nodeArray)
    {
        QJsonObject nodeObj = value.toObject();
        const QByteArray nodeId = nodeObj.value("id").toString().toLatin1();
        if(!m_impl->library->allowsNodeInGraph(nodeId, graphType))
            continue;
        Node *node = m_impl->library->createNode(nodeId);
        if(!node)
            continue;

        // A node's uniqueId round-trips through its json; a fresh one keeps
        // the paste from colliding with the original (findNode() and saved
        // connections key on it).
        const QString oldId = nodeObj.value("uniqueId").toString();
        nodeObj.insert("uniqueId", QString(QUuid::createUuid().toByteArray(QUuid::WithoutBraces)));
        node->readFromJson(nodeObj, m_impl->library);
        node->setPosition(node->position() + shift);

        m_impl->graph->addNode(node);
        pastedById.insert(oldId, node);
    }
    if(pastedById.isEmpty())
        return;

    for(const auto &value : root.value("connections").toArray())
    {
        const QJsonObject connection = value.toObject();
        Node *outNode = pastedById.value(connection.value("outputNode").toString());
        Node *inNode = pastedById.value(connection.value("inputNode").toString());
        if(!outNode || !inNode)
            continue;
        Parameter *outParam = outNode->findParameter(connection.value("outputParameter").toString().toLatin1());
        Parameter *inParam = inNode->findParameter(connection.value("inputParameter").toString().toLatin1());
        if(outParam && inParam)
            m_impl->graph->connectParameters(outParam, inParam);
    }

    // Queued edits; apply now so the items exist to select (as a Ctrl-drag
    // clone does).
    m_impl->graph->drainCommandQueue();

    clearSelection();
    for(Node *node : pastedById)
    {
        if(auto *item = itemForNode(node))
            item->setSelected(true);
    }
}

void Scene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    QGraphicsScene::contextMenuEvent(contextMenuEvent);

    if(contextMenuEvent->isAccepted())
        return;

    QMenu menu;

    QMenu *rootMenu = menu.addMenu("Add Node");

    const QPointF clickPos = contextMenuEvent->scenePos();
    menu.addSeparator();
    QAction *cutAction = menu.addAction("Cut", [this](){ cutSelectedNodes(); });
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setEnabled(hasSelectedNodes());
    QAction *copyAction = menu.addAction("Copy", [this](){ copySelectedNodes(); });
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(hasSelectedNodes());
    QAction *pasteAction = menu.addAction("Paste", [this, clickPos](){ pasteNodes(clickPos); });
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(clipboardHasNodes());

    auto graphType = graph()->graphTypeId();
    auto treeRoot = m_impl->library->createNodeTree([graphType](const keira::NodeInformation &info){return info.graphs.isEmpty() || info.graphs.contains(graphType);});

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

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if(m_impl->externalDropInterpreter && !m_impl->externalDropInterpreter(event->mimeData()).isEmpty())
        event->acceptProposedAction();
    else
        QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if(m_impl->externalDropInterpreter && !m_impl->externalDropInterpreter(event->mimeData()).isEmpty())
        event->acceptProposedAction();
    else
        QGraphicsScene::dragMoveEvent(event);
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    const QVector<ExternalDropNodeSpec> specs = m_impl->externalDropInterpreter ? m_impl->externalDropInterpreter(event->mimeData()) : QVector<ExternalDropNodeSpec>();

    if(specs.isEmpty() || !m_impl->library || !m_impl->graph)
    {
        QGraphicsScene::dropEvent(event);
        return;
    }

    QPointF position = event->scenePos();
    for(const auto &spec : specs)
    {
        Node *node = m_impl->library->createNode(spec.nodeId);
        if(!node)
            continue;

        node->setPosition(position);
        position += QPointF(24, 24);   // stagger multiple dropped nodes so they don't stack exactly on top of each other
        m_impl->graph->addNode(node);

        if(!spec.paramName.isEmpty())
            if(auto *param = node->findParameter(spec.paramName))
                param->setValue(spec.paramValue);
    }

    event->acceptProposedAction();
}

} // namespace keira
