#include "laserstatenode.h"

namespace photon {

keira::NodeInformation LaserStateNode::info()
{
    keira::NodeInformation toReturn([](){return new LaserStateNode;});
    toReturn.name = "Laser State";
    toReturn.nodeId = "photon.node.laser-state";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"bus", "surface", "dmx-subgraph", "routine", "fixture"};
    return toReturn;
}

LaserStateNode::LaserStateNode() : FixtureStateNode("photon.node.laser-state", "Laser State")
{
}

QVector<FixtureStateNode::CapabilityOption> LaserStateNode::addableCapabilities() const
{
    return {
        {"Content", Capability_LaserContent},
        {"Dimmer", Capability_Dimmer},
        {"Size", Capability_LaserSize},
        {"Position", Capability_LaserPosition},
        {"Rotation", Capability_LaserRotation},
        {"Color", Capability_LaserColor},
        {"Scan Rate", Capability_LaserScanRate},
        {"Visible Points", Capability_LaserVisiblePoints},
        {"Strobe", Capability_LaserStrobe},
    };
}

} // namespace photon
