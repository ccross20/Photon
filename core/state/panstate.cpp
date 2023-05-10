#include "panstate.h"
#include "fixture/capability/pancapability.h"

namespace photon {

PanState::PanState() : StateCapability(CapabilityType::Capability_Pan)
{
    setName("Pan");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Pan","How much to pan",0));
}

void PanState::evaluate(const StateEvaluationContext &t_context) const
{
    auto tilts = getFixtureCapabilities(t_context);
    double tiltPercent = getChannelFloat(t_context,0);

    for(auto curTilt : tilts)
    {
        auto pan = static_cast<PanCapability*>(curTilt);
        pan->setAnglePercent(tiltPercent,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon

