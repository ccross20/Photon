#include "tiltanglecenteredstate.h"
#include "fixture/capability/anglecapability.h"

namespace photon {

TiltAngleCenteredState::TiltAngleCenteredState() : StateCapability(CapabilityType::Capability_TiltAngleCentered)
{
    setName("Tilt Angle Centered");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Tilt Angle Centered","How much to tilt",0));
}

CapabilityType TiltAngleCenteredState::fixtureCapabilityType() const
{
    return CapabilityType::Capability_Tilt;
}

void TiltAngleCenteredState::evaluate(const StateEvaluationContext &t_context) const
{
    auto tilts = getFixtureCapabilities(t_context);
    double tiltPercent = getChannelFloat(t_context,0);

    for(auto curTilt : tilts)
    {
        auto tilt = static_cast<AngleCapability*>(curTilt);
        tilt->setAngleDegreesCentered(tiltPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon

