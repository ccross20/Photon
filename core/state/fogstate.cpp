#include "fogstate.h"
#include "fixture/capability/fogcapability.h"

namespace photon {

FogState::FogState() : StateCapability(CapabilityType::Capability_Fog)
{
    setName("Fog");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "On", "Fog output on", false));
}

void FogState::evaluate(const StateEvaluationContext &t_context) const
{
    const bool on = getChannelBool(t_context, 0);
    for(auto *capability : getFixtureCapabilities(t_context))
        static_cast<FogCapability*>(capability)->setOn(on, t_context.dmxMatrix);
}

} // namespace photon
