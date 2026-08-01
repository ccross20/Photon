#include "graphoutputnode.h"
#include "graph.h"
#include "parameter/parameter.h"

namespace keira {

GraphOutputNode::GraphOutputNode(const QByteArray &t_id) : Node(t_id)
{
}

GraphOutputNode::~GraphOutputNode()
{
}

QByteArray GraphOutputNode::portId() const
{
    return uniqueId();
}

Parameter *GraphOutputNode::valuePort() const
{
    if(m_valuePortId.isEmpty())
        return nullptr;
    return findParameter(m_valuePortId);
}

void GraphOutputNode::setValuePortId(const QByteArray &t_id)
{
    m_valuePortId = t_id;
}

QString GraphOutputNode::portName() const
{
    if(Parameter *port = valuePort())
        return port->name();
    return name();
}

QVariant GraphOutputNode::portValue() const
{
    if(Parameter *port = valuePort())
        return port->value();
    return QVariant();
}

void GraphOutputNode::notifyInterfaceChanged()
{
    if(graph())
        graph()->notifyInterfaceChanged();
}

} // namespace keira
