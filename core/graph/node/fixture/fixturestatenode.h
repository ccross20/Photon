#ifndef FIXTURESTATENODE_H
#define FIXTURESTATENODE_H

#include <deque>
#include <QHash>
#include "model/node.h"
#include "photon-global.h"

namespace keira { class BooleanParameter; }

namespace photon {

class FixtureListParameter;
class StateCapability;
class Fixture;

// A fixture-state node that applies a State (a set of capability values) to
// fixtures. It holds many capabilities at once, unlike the per-capability
// Set-Fixture nodes. Its capabilities are managed through a custom editor
// widget; individual capabilities can be exposed as graph input parameters so
// gizmos and other nodes can drive them.
//
// It runs in two modes, chosen by whether the evaluation context names a
// fixture (see evaluate()):
//  - Standalone (bus/surface/routine graphs): iterates its own fixture-list
//    input and staggers each fixture by its offset through a value history.
//  - Per-fixture (inside a fixture subgraph): the subgraph has already picked
//    the fixture and applied its time offset, so this applies to that one
//    fixture and leaves the offsetting alone.
class PHOTONCORE_EXPORT FixtureStateNode : public keira::Node
{
public:
    const static QByteArray Fixtures;
    const static QByteArray Enable;

    struct EnableSample { double time; bool enabled; };
    struct ValueSample { double time; QVariant value; };

    FixtureStateNode();
    ~FixtureStateNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    QWidget *createCustomWidget(keira::NodeEditor *) override;

    static keira::NodeInformation info();

    State *state() const;
    // Call after an in-place edit to the State (a capability value, or adding /
    // removing a capability) made through the custom editor. The State is not a
    // keira Parameter, so nothing else marks this node dirty - and without that,
    // a FixtureSubGraphNode keeps evaluating the stale per-fixture clones of
    // this node until an unrelated parameter change re-clones its pool (the
    // "have to disable/re-enable the node" symptom).
    void markStateEdited();
    // The node's currently assigned fixtures (resolved from the Fixtures input by
    // id), for editors that need to know what's targeted - e.g. to offer a
    // dropdown of a capability's available channel names on those fixtures.
    QVector<Fixture*> resolvedFixtures() const;

    // Expose a capability channel as a graph input port (an AnyParameter whose
    // id is the capability's channelId), so a gizmo can drive it. When exposed
    // and connected, the incoming value overrides the static one.
    bool isChannelExposed(StateCapability *, int channelIndex) const;
    void setChannelExposed(StateCapability *, int channelIndex, bool exposed);

    void readFromJson(const QJsonObject &, keira::NodeLibrary *) override;
    void writeToJson(QJsonObject &) const override;

private:
    // Applies the state to one fixture, reading exposed channels from `values`.
    void applyToFixture(struct RoutineEvaluationContext &, Fixture *,
                        const QHash<QByteArray, QVariant> &overrides) const;
    // Display label for an exposed channel's port: the channel's own name,
    // qualified by the capability so two channels of one capability (and
    // same-named channels of different capabilities) stay tellable apart.
    static QString portNameFor(StateCapability *, int channelIndex);

    FixtureListParameter *m_fixturesParam = nullptr;
    keira::BooleanParameter *m_enableParam = nullptr;
    State *m_state = nullptr;

    // History of the Enable value, so each fixture can be enabled/disabled at
    // its own offset — toggling Enable staggers across fixtures.
    mutable std::deque<EnableSample> m_enableHistory;
    // Per exposed-input value history (keyed by parameter id == channelId), for
    // the same per-fixture offset delay on driven capability values.
    mutable QHash<QByteArray, std::deque<ValueSample>> m_inputHistory;
    // Unexposed parameters are moved here rather than deleted immediately, so a
    // parameter is never freed while the eval thread might still reference it.
    QVector<keira::Parameter*> m_retiredParams;
};

} // namespace photon

#endif // FIXTURESTATENODE_H
