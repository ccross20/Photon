#include "uvstate.h"
#include "fixture/capability/uvcapability.h"

namespace photon {

UVState::UVState() : StateCapability(CapabilityType::Capability_UV)
{
    setName("UV");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "UV", "UV / Blacklight", 0.0, 0.0, 1.0));
}

void UVState::evaluate(const StateEvaluationContext &t_context) const
{
    auto uvs = getFixtureCapabilities(t_context);
    double uvPercent = getChannelFloat(t_context, 0);

    for(auto curUv : uvs)
    {
        auto uv = static_cast<UVCapability*>(curUv);
        uv->setPercent(uvPercent, t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
