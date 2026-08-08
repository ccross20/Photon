#include "graph_p.h"
#include "parameter/parameter_p.h"
#include "parameter/anyparameter.h"
#include "graphsorter.h"
#include "graphinputnode.h"
#include "graphoutputnode.h"
#include "node_p.h"
#include "library/nodelibrary.h"

namespace keira {

Graph::Graph(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
}

Graph::~Graph()
{
    for(Node *node : m_impl->nodes)
        delete node;
    delete m_impl;
}


QByteArray Graph::graphTypeId() const
{
    return m_impl->graphTypeId;
}

void Graph::setGraphTypeId(const QByteArray &t_id)
{
    m_impl->graphTypeId = t_id;
}

void Graph::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString Graph::name() const
{
    return m_impl->name;
}

QString Graph::familyName() const
{
    if(m_impl->parentNode)
        return m_impl->parentNode->graph()->familyName() + " > " + m_impl->name;
    else
        return m_impl->name;
}

Node *Graph::parentNode() const
{
    return m_impl->parentNode;
}

void Graph::setParentNode(Node *t_node)
{
    m_impl->parentNode = t_node;
}

QByteArray Graph::uniqueId() const
{
    return m_impl->uniqueId;
}

// ─────────────────────────────────────────────────────────────────────────────
// Command queue
// ─────────────────────────────────────────────────────────────────────────────

void Graph::drainCommandQueue()
{
    QVector<std::function<void()>> commands;
    {
        QMutexLocker lock(&m_impl->commandMutex);
        commands = std::move(m_impl->pendingCommands);
    }

    QVector<Node*> nodesSnapshot;
    bool interfaceDirty = false;
    {
        // Commands (addNodeInternal/removeNodeInternal/connectParametersInternal's
        // resort, ...) mutate `nodes` - hold the lock across them and take the
        // post-mutation snapshot in the same critical section, so evaluate() et al.
        // never see a half-applied node list.
        QMutexLocker lock(&m_impl->nodesMutex);
        for (auto &cmd : commands)
            cmd();
        nodesSnapshot = m_impl->nodes;
        interfaceDirty = m_impl->interfaceDirty;
        m_impl->interfaceDirty = false;
    }

    // Emitted here, after nodesMutex is released, rather than inline from
    // addNodeInternal()/removeNodeInternal() while still holding it. When the
    // graph and its FixtureClip facade live on the same thread as this call
    // (e.g. drainCommandQueue() forced synchronously from Sequence::save()),
    // interfaceChanged's connection to FixtureClip::syncChannelsFromGraph() is
    // direct, not queued - emitting it while nodesMutex is still locked meant
    // syncChannelsFromGraph()'s own inputPorts() call recursed straight back
    // into a lock this same thread already held, deadlocking against itself.
    // (When sender and receiver are on different threads the connection is
    // queued instead, so the receiving thread's lock attempt merely had to wait
    // its turn - still worth avoiding, but not a guaranteed deadlock like this.)
    if (interfaceDirty)
        emit interfaceChanged();

    // Recurse into subgraph nodes after our own commands run, so any
    // nodes added by those commands are included in the traversal. Iterate the
    // snapshot, not m_impl->nodes, so this doesn't hold nodesMutex across
    // arbitrary/virtual node code.
    for (Node *node : nodesSnapshot)
        node->drainCommandQueue();
}

// ─────────────────────────────────────────────────────────────────────────────
// Public mutation API — enqueue to be applied at next frame start
// ─────────────────────────────────────────────────────────────────────────────

void Graph::runCommand(std::function<void()> t_command)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back(std::move(t_command));
}

void Graph::addNode(Node *t_node)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_node]() { addNodeInternal(t_node); });
}

void Graph::removeNode(Node *t_node)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_node]() { removeNodeInternal(t_node); });
}

void Graph::connectParameters(const QByteArray &t_output, const QByteArray &t_input)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_output, t_input]() {
        // findParameterInternal(), not findParameter(): this command runs from
        // inside drainCommandQueue()'s nodesMutex-locked loop - findParameter()
        // would try to lock the same (non-recursive) mutex again and deadlock.
        Parameter *outParam = findParameterInternal(t_output);
        Parameter *inParam  = findParameterInternal(t_input);
        if (!outParam) { qWarning() << "Could not find output parameter:" << t_output; return; }
        if (!inParam)  { qWarning() << "Could not find input parameter:"  << t_input;  return; }
        connectParametersInternal(outParam, inParam);
    });
}

void Graph::disconnectParameters(const QByteArray &t_output, const QByteArray &t_input)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_output, t_input]() {
        // See connectParameters() above - must use the non-locking internal lookup.
        Parameter *outParam = findParameterInternal(t_output);
        Parameter *inParam  = findParameterInternal(t_input);
        if (!outParam) { qWarning() << "Could not find output parameter:" << t_output; return; }
        if (!inParam)  { qWarning() << "Could not find input parameter:"  << t_input;  return; }
        disconnectParametersInternal(outParam, inParam);
    });
}

void Graph::connectParameters(Parameter *t_output, Parameter *t_input)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_output, t_input]() {
        connectParametersInternal(t_output, t_input);
    });
}

void Graph::disconnectParameters(Parameter *t_output, Parameter *t_input)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_output, t_input]() {
        disconnectParametersInternal(t_output, t_input);
    });
}

void Graph::disconnectNode(Node *t_node)
{
    QMutexLocker lock(&m_impl->commandMutex);
    m_impl->pendingCommands.push_back([this, t_node]() { disconnectNodeInternal(t_node); });
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal implementations — only called from the eval thread via drainCommandQueue()
// or from other internal methods. No locking needed here.
// ─────────────────────────────────────────────────────────────────────────────

void Graph::addNodeInternal(Node *t_node)
{
    if(!t_node)
        return;
    if(m_impl->nodes.contains(t_node))
        return;
    m_impl->nodes.append(t_node);
    t_node->m_impl->graph = this;
    t_node->addedToGraph(this);

    nodeAdded(t_node);
    emit nodeWasAdded(t_node);

    // Deferred to drainCommandQueue(), after nodesMutex is released - see the
    // comment there.
    if(dynamic_cast<GraphInputNode*>(t_node) || dynamic_cast<GraphOutputNode*>(t_node))
        m_impl->interfaceDirty = true;

    markDirty(Dirty_Structure);
}

void Graph::removeNodeInternal(Node *t_node)
{
    if(!t_node)
        return;
    if(!m_impl->nodes.contains(t_node))
        return;
    disconnectNodeInternal(t_node);
    m_impl->nodes.removeOne(t_node);
    nodeRemoved(t_node);
    emit nodeWasRemoved(t_node);

    // Deferred to drainCommandQueue(), after nodesMutex is released - see the
    // comment there.
    if(dynamic_cast<GraphInputNode*>(t_node) || dynamic_cast<GraphOutputNode*>(t_node))
        m_impl->interfaceDirty = true;

    markDirty(Dirty_Structure);
}

void Graph::connectParametersInternal(Parameter *t_output, Parameter *t_input)
{
    if (!t_input->acceptsConnectionFrom(t_output)) {
        qWarning() << "Type mismatch: cannot connect"
                   << t_output->typeId() << "→" << t_input->typeId();
        return;
    }

    if(t_input->hasInput())
        disconnectParametersInternal(t_input->inputParameter(), t_input);

    t_output->m_impl->connectOutput(t_input);
    t_input->m_impl->connectInput(t_output);

    t_output->node()->outputParameterConnected(t_input);
    t_input->node()->inputParameterConnected(t_output);

    resortGraphInternal();

    emit parametersWereConnected(t_output, t_input);
}

void Graph::disconnectParametersInternal(Parameter *t_output, Parameter *t_input)
{
    t_output->m_impl->disconnectOutput(t_input);
    t_input->m_impl->disconnectInput(t_output);

    t_output->node()->outputParameterDisconnected(t_input);
    t_input->node()->inputParameterDisconnected(t_output);

    resortGraphInternal();

    emit parametersWereDisconnected(t_output, t_input);
}

void Graph::disconnectNodeInternal(Node *t_node)
{
    for(Parameter *param : t_node->parameters())
    {
        for(Parameter *outputParam : param->inputParameters())
            disconnectParametersInternal(outputParam, param);
        for(Parameter *inputParam : param->outputParameters())
            disconnectParametersInternal(param, inputParam);
    }
}

void Graph::resortGraphInternal()
{
    GraphSorter sorter(m_impl->nodes);
    m_impl->nodes = sorter.sorted();
}

// ─────────────────────────────────────────────────────────────────────────────
// Read-only queries and evaluation. The per-tick hot path (prepForEvaluation,
// evaluate, evaluateAll, markClean, inputPorts, outputPorts - the last two via
// SubGraphNode::applyInputs/readOutputs, called every tick from evaluate())
// snapshots `nodes` under nodesMutex before iterating, since drainCommandQueue()
// can be forced from the GUI thread out-of-band (see
// SubGraphNode::exposeInputForType) while the eval thread is concurrently
// mid-tick on the same graph. The remaining queries here are only ever called
// from whichever thread already owns the relevant graph/editor and are left
// unlocked, same as before.
// ─────────────────────────────────────────────────────────────────────────────

void Graph::nodeAdded(keira::Node *)   {}
void Graph::nodeRemoved(keira::Node *) {}

QVector<GraphInputNode*> Graph::inputPorts() const
{
    // Also hot-path: called every tick from SubGraphNode::applyInputs() on the
    // eval thread, on a graph an editor action (e.g. exposeInputForType) can
    // mutate from the GUI thread at the same time - needs the same protection
    // as evaluate() et al., not just the editor-only queries below.
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    QVector<GraphInputNode*> results;
    for(Node *node : nodesSnapshot)
        if(auto *input = dynamic_cast<GraphInputNode*>(node))
            results.append(input);
    return results;
}

QVector<GraphOutputNode*> Graph::outputPorts() const
{
    // See inputPorts() - called every tick from SubGraphNode::readOutputs() on
    // the eval thread, same race.
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    QVector<GraphOutputNode*> results;
    for(Node *node : nodesSnapshot)
        if(auto *output = dynamic_cast<GraphOutputNode*>(node))
            results.append(output);
    return results;
}

void Graph::notifyInterfaceChanged()
{
    emit interfaceChanged();
}

const QVector<Node*> &Graph::nodes() const
{
    return m_impl->nodes;
}

void Graph::updateNodePosition(Node *t_node)
{
    emit nodePositionUpdated(t_node);
}

Node *Graph::findNode(const QByteArray &t_query) const
{
    // Unlike its siblings (evaluate(), markClean(), inputPorts(), ...), this
    // used to read m_impl->nodes directly - the one gap in an otherwise
    // consistently-guarded class. Any caller resolving a node by name/id from
    // a thread other than this graph's own eval thread (e.g. a render loop
    // reaching into a clip's content graph while its editor's Scene is
    // concurrently draining structural edits on another thread) could race
    // an add/removeNodeInternal() and corrupt the vector mid-iteration.
    //
    // findNodeInternal() is the same search without the lock - drainCommandQueue()
    // already holds nodesMutex for its whole command loop, and connectParameters()/
    // disconnectParameters()'s queued (by-name) commands resolve their nodes via
    // findParameterInternal() from inside that loop; going through this locking
    // entry point there would self-deadlock (QMutex isn't recursive).
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    for(auto node : nodesSnapshot)
    {
        auto result = node->findNode(t_query);
        if(result)
            return result;
    }

    return nullptr;
}

Node *Graph::findNodeInternal(const QByteArray &t_query) const
{
    for(auto node : m_impl->nodes)
    {
        auto result = node->findNode(t_query);
        if(result)
            return result;
    }

    return nullptr;
}

QVector<Node *> Graph::nodeHierarchy() const
{
    if(parentNode())
        return parentNode()->nodeHierarchy();
    return QVector<Node *>();
}

Parameter *Graph::findParameter(const QByteArray &t_query)
{
    auto terms = t_query.split('/');
    Node *node = findNode(terms.front());
    if(node)
        return node->findParameter(terms.back());
    return nullptr;
}

Parameter *Graph::findParameterInternal(const QByteArray &t_query)
{
    auto terms = t_query.split('/');
    Node *node = findNodeInternal(terms.front());
    if(node)
        return node->findParameter(terms.back());
    return nullptr;
}

void Graph::prepForEvaluation()
{
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        if(m_impl->dirty & Dirty_Priority)
            resortGraphInternal();
        nodesSnapshot = m_impl->nodes;
    }

    for(Node *node : nodesSnapshot)
        node->prepForEvaluation();
}

void Graph::evaluate(EvaluationContext *t_context) const
{
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    for(Node *node : nodesSnapshot)
    {
        if(node->isDirty())
            node->evaluate(t_context);
    }
}

void Graph::evaluateAll(EvaluationContext *t_context) const
{
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    for(Node *node : nodesSnapshot)
        node->evaluate(t_context);
}

void Graph::markDirty(int t_dirty)
{
    if(m_impl->dirty & t_dirty)
        return;
    m_impl->dirty |= t_dirty;

    emit dirtyStateChanged();

    if(m_impl->parentNode)
        m_impl->parentNode->markDirty(t_dirty);
}

void Graph::markClean()
{
    m_impl->dirty = Dirty_Eval;

    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    for(Node *node : nodesSnapshot)
        node->markClean();
}

bool Graph::isDirty() const
{
    return m_impl->dirty != Clean;
}

void Graph::readFromJson(const QJsonObject &t_json, NodeLibrary *library)
{
    auto nodeArray = t_json.value("nodes").toArray();
    for(const auto &nodeJson : nodeArray)
    {
        const auto &nodeObj = nodeJson.toObject();
        Node *node = library->createNode(nodeObj.value("id").toString().toLatin1());

        if(node)
        {
            node->readFromJson(nodeObj, library);
            m_impl->nodes.append(node);
            node->m_impl->graph = this;
            node->addedToGraph(this);
        }
    }

    auto connectionArray = t_json.value("connections").toArray();
    for(const auto &connectionJson : connectionArray)
    {
        const auto &connectionObj = connectionJson.toObject();
        Node *outputNode = findNode(connectionObj.value("outputNode").toString().toLatin1());
        Node *inputNode  = findNode(connectionObj.value("inputNode").toString().toLatin1());

        if(!outputNode || !inputNode)
            continue;
        Parameter *outputParam = outputNode->findParameter(connectionObj.value("outputParameter").toString().toLatin1());
        Parameter *inputParam  = inputNode->findParameter(connectionObj.value("inputParameter").toString().toLatin1());

        if(!outputParam || !inputParam)
            continue;

        connectParametersInternal(outputParam, inputParam);
    }

    m_impl->graphTypeId = t_json["graphTypeId"].toString(m_impl->graphTypeId).toLatin1();
    m_impl->uniqueId    = t_json["uniqueId"].toString(m_impl->uniqueId).toLatin1();
    m_impl->name        = t_json["name"].toString();

    emit interfaceChanged();
}

void Graph::writeToJson(QJsonObject &t_json) const
{
    // Snapshot under nodesMutex like every other read path here (evaluate(),
    // findNode(), inputPorts(), ...) - this graph can be actively driven by an
    // eval thread (e.g. a FixtureClip wired into the Bus via a Sequence Node)
    // at the same moment something on another thread saves it; reading `nodes`
    // directly would race that thread's own drainCommandQueue()/evaluate()
    // mutating the same vector.
    QVector<Node*> nodesSnapshot;
    {
        QMutexLocker lock(&m_impl->nodesMutex);
        nodesSnapshot = m_impl->nodes;
    }

    QJsonArray nodeArray;
    QJsonArray connectionArray;

    for(auto node : nodesSnapshot)
    {
        QJsonObject nodeObj;
        node->writeToJson(nodeObj);
        nodeArray.append(nodeObj);

        for(auto param : node->parameters())
        {
            for(auto connectedParam : param->outputParameters())
            {
                QJsonObject connectionObj;
                connectionObj.insert("outputNode",      QString(node->uniqueId()));
                connectionObj.insert("outputParameter", QString(param->id()));
                connectionObj.insert("inputNode",       QString(connectedParam->node()->uniqueId()));
                connectionObj.insert("inputParameter",  QString(connectedParam->id()));
                connectionArray.append(connectionObj);
            }
        }
    }
    t_json.insert("uniqueId",   QString(m_impl->uniqueId));
    t_json.insert("name",       m_impl->name);
    t_json.insert("nodes",      nodeArray);
    t_json.insert("connections",connectionArray);
    t_json.insert("graphTypeId",QString(m_impl->graphTypeId));
}


} // namespace keira
