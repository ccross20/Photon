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
};

} // namespace photon

#endif // PHOTON_LASERSTATENODE_H
