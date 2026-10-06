#ifndef PHOTON_LASERSTATENODE_H
#define PHOTON_LASERSTATENODE_H

#include "fixturestatenode.h"

namespace photon {

// A FixtureStateNode whose capability menu offers laser controls (content,
// size, position, rotation, color, ...) instead of lighting ones. The mode
// channel is never exposed: arming and setup are owned by OutputOverridesNode.
class PHOTONCORE_EXPORT LaserStateNode : public FixtureStateNode
{
public:
    LaserStateNode();

    QVector<CapabilityOption> addableCapabilities() const override;

    static keira::NodeInformation info();

protected:
    // A clip's strength fades the laser through its Dimmer only: content,
    // size, position and the rest are written as set rather than blended
    // part-way, which for a page/cue number or a position is meaningless.
    bool strengthApplies(CapabilityType type) const override { return type == Capability_Dimmer; }
};

} // namespace photon

#endif // PHOTON_LASERSTATENODE_H
