#include "tiltstate.h"
#include "fixture/capability/anglecapability.h"

namespace photon {

TiltState::TiltState() : StateCapability(CapabilityType::Capability_Tilt)
{
    setName("Tilt");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Tilt","How much to tilt",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Angles","Use angles in degrees",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Centered","Centered",0));
}

void TiltState::evaluate(const StateEvaluationContext &t_context) const
{
    const auto tilts = getFixtureCapabilities(t_context);
    const double value = getChannelFloat(t_context, 0);
    const bool useAngles = getChannelBool(t_context, 1);
    const bool useCentered = getChannelBool(t_context, 2);

    for(auto *cap : tilts)
    {
        auto *tilt = static_cast<AngleCapability*>(cap);

        if(useAngles)
        {
            // Input is degrees. Centered: 0 = level; otherwise absolute within
            // the fixture's tilt range.
            if(useCentered)
                tilt->setAngleDegreesCentered(value, t_context.dmxMatrix, t_context.strength);
            else
                tilt->setAngleDegrees(value, t_context.dmxMatrix, t_context.strength);
        }
        else
        {
            // Input is a fraction. Plain: 0..1 -> DMX 0..255. Centered: -1..1 ->
            // DMX 0..255 with 0 landing at the middle.
            if(useCentered)
                tilt->setAnglePercentCentered(value, t_context.dmxMatrix, t_context.strength);
            else
                tilt->setAnglePercent(value, t_context.dmxMatrix, t_context.strength);
        }
        return;
    }
}

} // namespace photon
