#include "panstate.h"
#include "fixture/capability/anglecapability.h"

namespace photon {

PanState::PanState() : StateCapability(CapabilityType::Capability_Pan)
{
    setName("Pan");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Pan","How much to pan",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Angles","Use angles in degrees",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Centered","Centered",0));
}

void PanState::evaluate(const StateEvaluationContext &t_context) const
{
    const auto pans = getFixtureCapabilities(t_context);
    const double value = getChannelFloat(t_context, 0);
    const bool useAngles = getChannelBool(t_context, 1);
    const bool useCentered = getChannelBool(t_context, 2);

    for(auto *cap : pans)
    {
        auto *pan = static_cast<AngleCapability*>(cap);

        if(useAngles)
        {
            // Input is degrees. Centered: 0 = straight ahead; otherwise absolute
            // within the fixture's pan range.
            if(useCentered)
                pan->setAngleDegreesCentered(value, t_context.dmxMatrix, t_context.strength);
            else
                pan->setAngleDegrees(value, t_context.dmxMatrix, t_context.strength);
        }
        else
        {
            // Input is a fraction. Plain: 0..1 -> DMX 0..255. Centered: -1..1 ->
            // DMX 0..255 with 0 landing at the middle.
            if(useCentered)
                pan->setAnglePercentCentered(value, t_context.dmxMatrix, t_context.strength);
            else
                pan->setAnglePercent(value, t_context.dmxMatrix, t_context.strength);
        }
        return;
    }
}

} // namespace photon

