#include "graphsorter.h"
#include "node_p.h"
#include "parameter/parameter.h"

namespace keira {


GraphSorter::GraphSorter(NodeVector toSort)
{
    for(Node *node : toSort)
    {
        node->m_impl->visited = false;
        m_toSort.prepend(node);
    }


    bool foundNode = true;
    while(foundNode)
    {
        foundNode = false;
        for(Node *node : m_toSort)
        {
            if(!node->m_impl->visited)
            {
                foundNode = true;
                visit(node);
                break;
            }
        }
    }

    /*
    qDebug() << "Begin Nodes";
    for(auto node :m_sorted )
        qDebug() << node->name();

    qDebug() << "End Nodes";
*/

    std::sort(m_sorted.begin(), m_sorted.end(), [](const Node* a, const Node* b) {
        return a->priority() < b->priority();
    });
/*
    qDebug() << "Begin Sorted Nodes";
    for(auto node :m_sorted )
        qDebug() << node->name();

    qDebug() << "End Sorted Nodes";
*/
}

void GraphSorter::visit(Node *node)
{
    if(node->m_impl->visited)
        return;

    if(m_visiting.contains(node))
    {
        // Cycle (a node depends, directly or transitively, on itself) - the
        // graph isn't a DAG, so there's no correct position to give it. Stop
        // recursing here instead of looping forever; the connection that
        // caused this should really be rejected up where connections are
        // made (see Graph::connectParametersInternal), but this keeps a
        // malformed graph (e.g. loaded from an old/corrupt file) from being
        // able to crash the app either way.
        qWarning() << "GraphSorter: cycle detected at node" << node->name() << "- graph is not a DAG";
        return;
    }
    m_visiting.insert(node);

    for(Parameter *param : node->parameters())
    {
        for(Parameter *inputParam : param->outputParameters())
        {
            visit(inputParam->node());
        }
    }

    m_visiting.remove(node);
    node->m_impl->visited = true;
    m_sorted.push_front(node);
}

} // namespace exo

/*
 *
#include "graphsorter.h"
#include "node_p.h"
#include "network/port_p.h"

namespace exo {

GraphSorter::GraphSorter(NodeList toSort)
{
    for(NodePtr node : toSort)
    {
        node->m_impl->visited = false;

        for(PortPtr port : node->dataInputPorts() + node->parameterPorts())
            port->m_impl->visited = false;


        m_toSort.push_back(node);
    }

    bool foundNode = true;
    while(foundNode)
    {
        foundNode = false;
        for(NodePtr node : m_toSort)
        {
            if(visit(node))
            {
                foundNode = true;
                break;
            }
        }
    }
}

bool GraphSorter::visit(NodePtr node)
{
    if(node->m_impl->visited)
        return false;

    int counter = 0;
    for(PortPtr port : node->dataInputPorts() + node->parameterPorts())
    {
        if(port->isInput() && port->isConnected() && !port->m_impl->visited)
            return false;
        else if(port->isInput() && port->isConnected())
                ++counter;
    }

    node->m_impl->visited = true;
    m_sorted.push_back(node);


    for(PortPtr port : node->dataOutputPorts() + node->parameterPorts())
    {
        if(port->isInput())
            continue;

        for(PortPtr connected : port->connections())
        {
            connected->m_impl->visited = true;
        }
    }
    return true;
}

} // namespace exo

 */
