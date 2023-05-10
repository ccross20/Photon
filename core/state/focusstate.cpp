#include "focusstate.h"
#include "fixture/capability/focuscapability.h"

namespace photon {

FocusState::FocusState() : StateCapability(CapabilityType::Capability_Focus)
{
    setName("Focus");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Focus","How much to focus",0));
}

void FocusState::evaluate(const StateEvaluationContext &t_context) const
{
    auto tilts = getFixtureCapabilities(t_context);
    double tiltPercent = getChannelFloat(t_context,0);

    for(auto curTilt : tilts)
    {
        auto tilt = static_cast<FocusCapability*>(curTilt);
        tilt->setAnglePercent(tiltPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
