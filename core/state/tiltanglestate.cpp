#include "tiltanglestate.h"
#include "fixture/capability/anglecapability.h"

namespace photon {

TiltAngleState::TiltAngleState() : StateCapability(CapabilityType::Capability_TiltAngle)
{
    setName("Tilt Angle");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Tilt Angle","How much to tilt",0));
}

CapabilityType TiltAngleState::fixtureCapabilityType() const
{
    return CapabilityType::Capability_Tilt;
}

void TiltAngleState::evaluate(const StateEvaluationContext &t_context) const
{
    auto tilts = getFixtureCapabilities(t_context);
    double tiltPercent = getChannelFloat(t_context,0);

    for(auto curTilt : tilts)
    {
        auto tilt = static_cast<AngleCapability*>(curTilt);
        tilt->setAngleDegrees(tiltPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
