#ifndef KEIRA_GRAPHOUTPUTNODE_H
#define KEIRA_GRAPHOUTPUTNODE_H

#include "node.h"

namespace keira {

// Base for a node that declares one typed OUTPUT of its graph.
//
// Inside the graph the node presents an input "value" port that other nodes wire
// into. An enclosing SubGraphNode mirrors it as one of its own outer output
// parameters and copies the value back out after each evaluation
// (see SubGraphNode::readOutputs).
//
// Output ports are for VALUE-producing graphs (e.g. a canvas graph yielding a
// texture, or a reusable graph returning a number/color). A terminal graph — one
// whose effect is a side-effect on the evaluation context, such as a fixture
// graph writing DMX — simply declares no output ports.
//
// Matched to its outer mirror by portId(), defaulting to uniqueId().
class KEIRA_EXPORT GraphOutputNode : public Node
{
public:
    explicit GraphOutputNode(const QByteArray &id);
    ~GraphOutputNode() override;

    // Stable identifier matching this port to its mirror. Defaults to uniqueId().
    virtual QByteArray portId() const;

    // The input parameter inner nodes wire into; its typeId drives the type of
    // the mirrored outer parameter. Resolved by id so it survives load. Null if
    // not yet set.
    Parameter *valuePort() const;

    // Display name for editors and the mirrored outer parameter.
    virtual QString portName() const;

    // The value to publish on the outer mirror after evaluation. Default reads
    // the value port.
    virtual QVariant portValue() const;

protected:
    // Designate which parameter (by id) is the value port. Call from the subclass
    // constructor so it is set on both fresh and loaded instances.
    void setValuePortId(const QByteArray &);

    // Notify the enclosing graph that this port's metadata changed.
    void notifyInterfaceChanged();

private:
    QByteArray m_valuePortId;
};

} // namespace keira

#endif // KEIRA_GRAPHOUTPUTNODE_H
