#include "subgraphnode.h"
#include "graph.h"

namespace keira {

SubGraphNode::SubGraphNode(const QByteArray &id) :Node(id) {
    m_graph = new Graph();
    m_graph->setName("Subgraph");
    m_graph->setParentNode(this);
}

SubGraphNode::~SubGraphNode()
{
    delete m_graph;
}

Node *SubGraphNode::findNode(const QByteArray &t_query) const
{
    auto result = Node::findNode(t_query);

    if(result)
        return result;

    return m_graph->findNode(t_query);
}

void SubGraphNode::evaluate(EvaluationContext *t_context) const
{
    Node::evaluate(t_context);
    m_graph->evaluate(t_context);
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
