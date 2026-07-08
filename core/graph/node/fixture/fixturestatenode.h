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

// A fixture-state node that applies a State (a set of capability values) to a
// list of fixtures. Unlike the per-capability Set-Fixture nodes it holds many
// capabilities at once and iterates its own fixture-list input, so it lives in
// the surface/bus graph rather than inside a fixture subgraph. Its capabilities
// are managed through a custom editor widget; individual capabilities can later
// be exposed as graph input parameters so gizmos can drive them.
class PHOTONCORE_EXPORT FixtureStateNode : public keira::Node
{
public:
    const static QByteArray Fixtures;
    const static QByteArray Enable;

    struct EnableSample { double time; bool enabled; };

    FixtureStateNode();
    ~FixtureStateNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    QWidget *createCustomWidget(keira::NodeEditor *) override;

    static keira::NodeInformation info();

    State *state() const;

    // Expose a capability channel as a graph input port (an AnyParameter whose
    // id is the capability's channelId), so a gizmo can drive it. When exposed
    // and connected, the incoming value overrides the static one.
    bool isChannelExposed(StateCapability *, int channelIndex) const;
    void setChannelExposed(StateCapability *, int channelIndex, bool exposed);

    void readFromJson(const QJsonObject &, keira::NodeLibrary *) override;
    void writeToJson(QJsonObject &) const override;

private:
    struct ValueSample { double time; QVariant value; };

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
