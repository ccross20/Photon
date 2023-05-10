#include "zoomstate.h"
#include "fixture/capability/zoomcapability.h"

namespace photon {

ZoomState::ZoomState() : StateCapability(CapabilityType::Capability_Zoom)
{
    setName("Zoom");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Zoom","How much to zoom",0));
}

void ZoomState::evaluate(const StateEvaluationContext &t_context) const
{
    auto zooms = getFixtureCapabilities(t_context);
    double zoomPercent = getChannelFloat(t_context,0);

    for(auto curZoom : zooms)
    {
        auto zoom = static_cast<ZoomCapability*>(curZoom);
        zoom->setAnglePercent(zoomPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon

