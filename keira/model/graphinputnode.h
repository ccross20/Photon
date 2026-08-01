#ifndef KEIRA_GRAPHINPUTNODE_H
#define KEIRA_GRAPHINPUTNODE_H

#include "node.h"

namespace keira {

// Base for a node that declares one typed INPUT of its graph.
//
// Inside the graph the node presents an output "value" port that other nodes
// wire from. An enclosing SubGraphNode mirrors this port as one of its own outer
// input parameters and relays the outer value in before each evaluation
// (see SubGraphNode::applyInputs). A graph may have any number of these — or
// none, for a terminal graph.
//
// Ports are matched to their outer mirror by portId(), which defaults to the
// node's uniqueId(): persisted, stable across save/load, and preserved across
// the per-fixture pool clones a SubGraphNode may make (readFromJson keeps
// uniqueId). This is what lets us bind by id rather than by fragile index.
class KEIRA_EXPORT GraphInputNode : public Node
{
public:
    explicit GraphInputNode(const QByteArray &id);
    ~GraphInputNode() override;

    // Stable identifier matching this port to its mirror. Defaults to uniqueId().
    virtual QByteArray portId() const;

    // The output parameter inner nodes wire from; its typeId drives the type of
    // the mirrored outer parameter. Resolved by id, so it is valid whether the
    // node was freshly built or restored from JSON. Null if not yet set.
    Parameter *valuePort() const;

    // Display metadata for editors and the mirrored outer parameter.
    virtual QString portName() const;
    virtual QString portDescription() const;

    // Push the outer value in before evaluation. Default writes it onto the value
    // port so downstream nodes read it.
    virtual void setPortValue(const QVariant &);

protected:
    // Designate which parameter (by id) is the value port. Call from the subclass
    // constructor so it is set on both fresh and loaded instances.
    void setValuePortId(const QByteArray &);

    // Notify the enclosing graph that this port's metadata (name/type/default)
    // changed so mirrors and editors refresh. Membership changes (add/remove of
    // the node itself) are detected by the graph automatically.
    void notifyInterfaceChanged();

private:
    QByteArray m_valuePortId;
};

} // namespace keira

#endif // KEIRA_GRAPHINPUTNODE_H
