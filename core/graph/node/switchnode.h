#ifndef PHOTON_SWITCHNODE_H
#define PHOTON_SWITCHNODE_H
#include <QVector>
#include "model/node.h"
#include "photon-global.h"

namespace keira { class IntegerParameter; class AnyParameter; }

namespace photon {

// Forwards whichever input is currently selected (by index) to a single
// output. Every port (inputs and output alike) is untyped ("any") until the
// switch's type is decided by whichever connection happens first — an input
// being wired, or the output being wired to something downstream. Once
// decided, the output and every existing/future input all lock to that same
// type. There is always exactly one extra, empty input past the last
// connected one — wiring the last input grows the list by one; earlier
// inputs are never renumbered or removed, so disconnecting one just leaves a
// (locked) gap rather than shifting other indices.
class PHOTONCORE_EXPORT SwitchNode : public keira::Node
{
public:
    const static QByteArray Select;
    const static QByteArray Output;
    // Input parameter ids are InputPrefix + index, e.g. "input0", "input1", ...
    const static QByteArray InputPrefix;

    SwitchNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

    void readFromJson(const QJsonObject &, keira::NodeLibrary *) override;

protected:
    void inputParameterConnected(keira::Parameter *) override;
    void outputParameterConnected(keira::Parameter *) override;

private:
    keira::AnyParameter *makeInput(int index) const;
    void rebuildBookkeeping();
    void growIfNeeded();
    // Resolves whichever of our own ports (output, inputs) aren't already
    // resolved to t_type, then — only if that actually changed something —
    // spreads to directly-connected Switch neighbors. Calling this on an
    // already-fully-resolved node is a safe no-op, which is what keeps the
    // neighbor spread from looping forever around a connected mesh.
    void applyType(const QByteArray &type);
    // Called from both connection hooks: works out which side (ours or the
    // newly-connected neighbor's) already has a concrete type and, if either
    // does, applies it to both — this is what lets an already-typed switch
    // push its type onto a switch it's *just now* being wired to, not only
    // one that was generic at connection time.
    void reconcileConnection(keira::Parameter *ours, keira::Parameter *theirs);

    keira::IntegerParameter *m_selectParam = nullptr;
    keira::AnyParameter *m_outputParam = nullptr;
    QVector<keira::AnyParameter*> m_inputs;
};

} // namespace photon

#endif // PHOTON_SWITCHNODE_H
