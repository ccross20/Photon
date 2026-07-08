#ifndef FIXTURESTATENODE_H
#define FIXTURESTATENODE_H

#include <deque>
#include "model/node.h"
#include "photon-global.h"

namespace keira { class BooleanParameter; }

namespace photon {

class FixtureListParameter;

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

    void readFromJson(const QJsonObject &, keira::NodeLibrary *) override;
    void writeToJson(QJsonObject &) const override;

private:
    FixtureListParameter *m_fixturesParam = nullptr;
    keira::BooleanParameter *m_enableParam = nullptr;
    State *m_state = nullptr;

    // History of the Enable value, so each fixture can be enabled/disabled at
    // its own offset — toggling Enable staggers across fixtures.
    mutable std::deque<EnableSample> m_enableHistory;
};

} // namespace photon

#endif // FIXTURESTATENODE_H
