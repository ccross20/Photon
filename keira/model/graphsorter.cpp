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
}

void GraphSorter::visit(Node *node)
{
    if(node->m_impl->visited)
        return;

    for(Parameter *param : node->parameters())
    {
        for(Parameter *inputParam : param->outputParameters())
        {
            visit(inputParam->node());
        }
    }

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
