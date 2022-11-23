#include "graph_p.h"
#include "parameter/parameter_p.h"
#include "graphsorter.h"
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

QByteArray Graph::uniqueId() const
{
    return m_impl->uniqueId;
}

void Graph::addNode(Node *t_node)
{
    if(m_impl->nodes.contains(t_node))
        return;
    m_impl->nodes.append(t_node);
    t_node->m_impl->graph = this;

    emit nodeWasAdded(t_node);

    markDirty();
}

void Graph::removeNode(Node *t_node)
{
    if(!m_impl->nodes.contains(t_node))
        return;
    disconnectNode(t_node);
    m_impl->nodes.removeOne(t_node);
    emit nodeWasRemoved(t_node);
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
    auto result = std::find_if(m_impl->nodes.cbegin(), m_impl->nodes.cend(),[t_query](Node* t_node){return t_node->name().toLatin1() == t_query;});
    if(result != m_impl->nodes.cend())
        return *result;

    result = std::find_if(m_impl->nodes.cbegin(), m_impl->nodes.cend(),[t_query](Node* t_node){return t_node->uniqueId() == t_query;});
    if(result != m_impl->nodes.cend())
        return *result;
    return nullptr;
}

Parameter *Graph::findParameter(const QByteArray &t_query)
{
    auto terms = t_query.split('/');
    Node *node = findNode(terms.front());
    if(node)
        return node->findParameter(terms.back());
    return nullptr;
}

void Graph::evaluate(EvaluationContext *t_context) const
{
    for(Node *node : m_impl->nodes)
    {
        if(node->isDirty())
        {
            node->evaluate(t_context);
        }
    }

}

void Graph::evaluateAll(EvaluationContext *t_context) const
{
    for(Node *node : m_impl->nodes)
    {
        node->evaluate(t_context);
    }

}

void Graph::connectParameters(const QByteArray &t_output, const QByteArray &t_input)
{
    Parameter *outParam = findParameter(t_output);
    Parameter *inParam = findParameter(t_input);

    if(!outParam)
    {
        qWarning() << "Could not find output parameter:" << t_output;
        return;
    }

    if(!inParam)
    {
        qWarning() << "Could not find input parameter:" << t_input;
        return;
    }

    connectParameters(outParam, inParam);
}

void Graph::disconnectParameters(const QByteArray &t_output, const QByteArray &t_input)
{
    Parameter *outParam = findParameter(t_output);
    Parameter *inParam = findParameter(t_input);

    if(!outParam)
    {
        qWarning() << "Could not find output parameter:" << t_output;
        return;
    }

    if(!inParam)
    {
        qWarning() << "Could not find input parameter:" << t_input;
        return;
    }

    disconnectParameters(outParam, inParam);
}

void Graph::connectParameters(Parameter *t_output, Parameter *t_input)
{
    if(t_input->hasInput())
        disconnectParameters(t_input->inputParameter(), t_input);


    t_output->m_impl->connectOutput(t_input);
    t_input->m_impl->connectInput(t_output);

    t_output->node()->outputParameterConnected(t_input);
    t_input->node()->inputParameterConnected(t_output);

    GraphSorter sorter(m_impl->nodes);
    m_impl->nodes = sorter.sorted();

    emit parametersWereConnected(t_output, t_input);
}

void Graph::disconnectParameters(Parameter *t_output, Parameter *t_input)
{
    t_output->m_impl->disconnectOutput(t_input);
    t_input->m_impl->disconnectInput(t_output);

    t_output->node()->outputParameterDisconnected(t_input);
    t_input->node()->inputParameterDisconnected(t_output);

    GraphSorter sorter(m_impl->nodes);
    m_impl->nodes = sorter.sorted();

    emit parametersWereDisconnected(t_output, t_input);
}

void Graph::disconnectNode(Node *t_node)
{
    for(Parameter *param : t_node->parameters())
    {
        for(Parameter *outputParam : param->inputParameters())
        {
            disconnectParameters(outputParam, param);
        }
        for(Parameter *inputParam : param->outputParameters())
        {
            disconnectParameters(param, inputParam);
        }
    }
}

void Graph::markDirty()
{
    if(m_impl->isDirty)
        return;
    m_impl->isDirty = true;

    emit dirtyStateChanged();
}

void Graph::markClean()
{
    m_impl->isDirty = false;

    for(Node *node : m_impl->nodes)
    {
        node->markClean();

    }
}

bool Graph::isDirty() const
{
    return m_impl->isDirty;
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
        }
    }

    auto connectionArray = t_json.value("connections").toArray();
    for(const auto &connectionJson : connectionArray)
    {
        const auto &connectionObj = connectionJson.toObject();
        Node *outputNode = findNode(connectionObj.value("outputNode").toString().toLatin1());
        Node *inputNode = findNode(connectionObj.value("inputNode").toString().toLatin1());

        if(!outputNode || !inputNode)
            continue;
        Parameter *outputParam = outputNode->findParameter(connectionObj.value("outputParameter").toString().toLatin1());
        Parameter *inputParam = inputNode->findParameter(connectionObj.value("inputParameter").toString().toLatin1());

        if(!outputParam || !inputParam)
            continue;

        connectParameters(outputParam, inputParam);
    }

    m_impl->uniqueId = t_json["uniqueId"].toString().toLatin1();
}

void Graph::writeToJson(QJsonObject &t_json) const
{
    QJsonArray nodeArray;
    QJsonArray connectionArray;

    for(auto node : m_impl->nodes)
    {
        QJsonObject nodeObj;
        node->writeToJson(nodeObj);
        nodeArray.append(nodeObj);

        for(auto param : node->parameters())
        {
            for(auto connectedParam : param->outputParameters())
            {
                QJsonObject connectionObj;
                connectionObj.insert("outputNode", QString(node->uniqueId()));
                connectionObj.insert("outputParameter", QString(param->id()));
                connectionObj.insert("inputNode", QString(connectedParam->node()->uniqueId()));
                connectionObj.insert("inputParameter", QString(connectedParam->id()));
                connectionArray.append(connectionObj);
            }
        }
    }
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    t_json.insert("nodes", nodeArray);
    t_json.insert("connections", connectionArray);
}


} // namespace keira
