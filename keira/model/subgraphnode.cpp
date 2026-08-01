#include "subgraphnode.h"
#include "graph.h"
#include "graphinputnode.h"
#include "graphoutputnode.h"
#include "parameter/parameter.h"
#include "library/nodelibrary.h"

namespace keira {

SubGraphNode::SubGraphNode(const QByteArray &id) :Node(id) {
    m_graph = new Graph();
    m_graph->setName("Subgraph");
    m_graph->setParentNode(this);

    // Re-mirror our outer parameters whenever the inner graph's interface changes.
    m_interfaceConn = QObject::connect(m_graph, &Graph::interfaceChanged,
                                       m_graph, [this]() { reconcilePorts(); });
}

SubGraphNode::~SubGraphNode()
{
    QObject::disconnect(m_interfaceConn);
    qDeleteAll(m_retiredMirrors);
    delete m_graph;
}

NodeLibrary *SubGraphNode::nodeLibrary() const
{
    return nullptr;
}

void SubGraphNode::drainCommandQueue()
{
    m_graph->drainCommandQueue();
}

Node *SubGraphNode::findNode(const QByteArray &t_query) const
{
    auto result = Node::findNode(t_query);

    if(result)
        return result;

    return m_graph->findNode(t_query);
}

// ─────────────────────────────────────────────────────────────────────────────
// Interface mirroring
// ─────────────────────────────────────────────────────────────────────────────

void SubGraphNode::reconcilePorts()
{
    NodeLibrary *library = nodeLibrary();
    if(!library)   // no library to clone typed parameters through (e.g. headless)
        return;

    // The actual parameter mutations must not race the evaluator, so run them at a
    // safe point on the outer graph's command queue when we're in a graph.
    auto reconcile = [this, library]() {
        // ── Inputs: one outer input parameter per input port ──
        QSet<QByteArray> wantedInputs;
        for(GraphInputNode *port : m_graph->inputPorts())
        {
            const QByteArray portId = port->portId();
            wantedInputs.insert(portId);

            Parameter *mirror = m_inputMirrors.value(portId);
            if(mirror)
            {
                mirror->setName(port->portName());
                continue;
            }
            // Adopt a param already present (e.g. restored from load) instead of
            // adding a duplicate.
            if(Parameter *existing = findParameter(portId))
            {
                existing->setName(port->portName());
                m_inputMirrors.insert(portId, existing);
                continue;
            }
            if(!port->valuePort())
                continue;

            mirror = port->valuePort()->clone(library);
            mirror->setId(portId);
            mirror->setName(port->portName());
            mirror->setConnectionOptions(AllowSingleInput | AllowMultipleOutput);
            addParameter(mirror);
            m_inputMirrors.insert(portId, mirror);
        }
        for(const QByteArray &portId : m_inputMirrors.keys())
            if(!wantedInputs.contains(portId))
                retireMirror(m_inputMirrors.take(portId));

        // ── Outputs: one outer output parameter per output port ──
        QSet<QByteArray> wantedOutputs;
        for(GraphOutputNode *port : m_graph->outputPorts())
        {
            const QByteArray portId = port->portId();
            wantedOutputs.insert(portId);

            Parameter *mirror = m_outputMirrors.value(portId);
            if(mirror)
            {
                mirror->setName(port->portName());
                continue;
            }
            if(Parameter *existing = findParameter(portId))
            {
                existing->setName(port->portName());
                m_outputMirrors.insert(portId, existing);
                continue;
            }
            if(!port->valuePort())
                continue;

            mirror = port->valuePort()->clone(library);
            mirror->setId(portId);
            mirror->setName(port->portName());
            mirror->setConnectionOptions(AllowMultipleOutput);
            addParameter(mirror);
            m_outputMirrors.insert(portId, mirror);
        }
        for(const QByteArray &portId : m_outputMirrors.keys())
            if(!wantedOutputs.contains(portId))
                retireMirror(m_outputMirrors.take(portId));

        portsChanged();
    };

    if(Node::graph())
        Node::graph()->runCommand(reconcile);
    else
        reconcile();
}

void SubGraphNode::retireMirror(Parameter *t_param)
{
    if(!t_param)
        return;

    // Drop any outer-graph connections before detaching the parameter.
    if(Node::graph())
    {
        const auto inputs = t_param->inputParameters();
        for(Parameter *in : inputs)
            Node::graph()->disconnectParameters(in, t_param);
        const auto outputs = t_param->outputParameters();
        for(Parameter *out : outputs)
            Node::graph()->disconnectParameters(t_param, out);
    }

    removeParameter(t_param);
    m_retiredMirrors.append(t_param);
}

Parameter *SubGraphNode::exposeInputForType(const QByteArray &t_paramTypeId)
{
    NodeLibrary *library = nodeLibrary();
    if(!library)
        return nullptr;

    const QByteArray nodeId = library->inputNodeForParameterType(t_paramTypeId);
    if(nodeId.isEmpty())
        return nullptr;

    auto *created = library->createNode(nodeId);
    auto *input = dynamic_cast<GraphInputNode*>(created);
    if(!input)
    {
        delete created;
        return nullptr;
    }

    // Add the input node, then drain so the interfaceChanged → reconcilePorts chain
    // creates the outer mirror before we return it. reconcilePorts queues onto the
    // outer graph when we're in one, so drain that too.
    m_graph->addNode(input);
    m_graph->drainCommandQueue();
    if(Node::graph())
        Node::graph()->drainCommandQueue();

    return findParameter(input->portId());
}

void SubGraphNode::applyInputs(Graph *t_target) const
{
    for(GraphInputNode *port : t_target->inputPorts())
    {
        auto it = m_inputMirrors.constFind(port->portId());
        if(it != m_inputMirrors.constEnd())
            port->setPortValue((*it)->value());
    }
}

void SubGraphNode::readOutputs(Graph *t_source) const
{
    for(GraphOutputNode *port : t_source->outputPorts())
    {
        auto it = m_outputMirrors.constFind(port->portId());
        if(it != m_outputMirrors.constEnd())
            (*it)->setValue(port->portValue());
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void SubGraphNode::evaluate(EvaluationContext *t_context) const
{
    Node::evaluate(t_context);
    m_graph->drainCommandQueue();
    applyInputs(m_graph);
    m_graph->evaluate(t_context);
    readOutputs(m_graph);
}

void SubGraphNode::markDirty(int t_dirty)
{
    Node::markDirty(t_dirty);

    m_graph->markDirty(t_dirty);
}

void SubGraphNode::markClean()
{
    Node::markClean();

    m_graph->markClean();
}

void SubGraphNode::readFromJson(const QJsonObject &t_json, NodeLibrary *t_library)
{
    Node::readFromJson(t_json, t_library);

    auto subgraph = t_json.value("subgraph").toObject();
    m_graph->readFromJson(subgraph, t_library);
}

void SubGraphNode::writeToJson(QJsonObject &t_json) const
{
    Node::writeToJson(t_json);

    QJsonObject subgraph;
    m_graph->writeToJson(subgraph);
    t_json.insert("subgraph", subgraph);

}

} // namespace keira
