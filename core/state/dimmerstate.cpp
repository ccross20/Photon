#include "dimmerstate.h"
#include "fixture/capability/dimmercapability.h"

namespace photon {

DimmerState::DimmerState() : StateCapability(CapabilityType::Capability_Dimmer)
{
    setName("Dimmer");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Dimmer","Lamp Brightness",1.0));
}

void DimmerState::evaluate(const StateEvaluationContext &t_context) const
{
    auto dimmers = getFixtureCapabilities(t_context);
    double dimmerPercent = getChannelFloat(t_context,0);

    for(auto curDim : dimmers)
    {
        auto dim = static_cast<DimmerCapability*>(curDim);
        dim->setPercent(dimmerPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
