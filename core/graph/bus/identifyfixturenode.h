#ifndef PHOTON_IDENTIFYFIXTURENODE_H
#define PHOTON_IDENTIFYFIXTURENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Sits last in the bus pipeline, right before the output node. When enabled,
// overrides one fixture's dimmer/shutter/color channels to full brightness,
// open shutter, and white so it can be spotted in the physical rig — driven
// directly by the DMX Patch panel's Identify toggle rather than manual wiring.
class PHOTONCORE_EXPORT IdentifyFixtureNode : public keira::Node
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;
    const static QByteArray FixtureParam;
    const static QByteArray EnabledParam;

    IdentifyFixtureNode();
    ~IdentifyFixtureNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    // Driven directly by the DMX Patch panel's Identify toggle and its
    // current selection, rather than by graph wiring.
    void setIdentifiedFixture(const QByteArray &fixtureId);
    void setIdentifyEnabled(bool enabled);
    bool isIdentifyEnabled() const;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_IDENTIFYFIXTURENODE_H
