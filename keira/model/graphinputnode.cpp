#include "graphinputnode.h"
#include "graph.h"
#include "parameter/parameter.h"

namespace keira {

GraphInputNode::GraphInputNode(const QByteArray &t_id) : Node(t_id)
{
}

GraphInputNode::~GraphInputNode()
{
}

QByteArray GraphInputNode::portId() const
{
    return uniqueId();
}

Parameter *GraphInputNode::valuePort() const
{
    if(m_valuePortId.isEmpty())
        return nullptr;
    return findParameter(m_valuePortId);
}

void GraphInputNode::setValuePortId(const QByteArray &t_id)
{
    m_valuePortId = t_id;
}

QString GraphInputNode::portName() const
{
    if(Parameter *port = valuePort())
        return port->name();
    return name();
}

QString GraphInputNode::portDescription() const
{
    if(Parameter *port = valuePort())
        return port->description();
    return QString();
}

void GraphInputNode::setPortValue(const QVariant &t_value)
{
    if(Parameter *port = valuePort())
        port->setValue(t_value);
}

void GraphInputNode::notifyInterfaceChanged()
{
    if(graph())
        graph()->notifyInterfaceChanged();
}

} // namespace keira
