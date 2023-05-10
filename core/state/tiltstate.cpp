#include "tiltstate.h"
#include "fixture/capability/tiltcapability.h"

namespace photon {

TiltState::TiltState() : StateCapability(CapabilityType::Capability_Tilt)
{
    setName("Tilt");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Tilt","How much to tilt",0));
}

void TiltState::evaluate(const StateEvaluationContext &t_context) const
{
    auto tilts = getFixtureCapabilities(t_context);
    double tiltPercent = getChannelFloat(t_context,0);

    for(auto curTilt : tilts)
    {
        auto tilt = static_cast<TiltCapability*>(curTilt);
        tilt->setAnglePercent(tiltPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
