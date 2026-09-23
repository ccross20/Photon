#include "ctostate.h"
#include "fixture/capability/ctocapability.h"

namespace photon {

CTOState::CTOState() : StateCapability(CapabilityType::Capability_CTO)
{
    setName("CTO");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "CTO", "Color Temperature Correction", 0.0, 0.0, 1.0));
}

void CTOState::evaluate(const StateEvaluationContext &t_context) const
{
    auto ctos = getFixtureCapabilities(t_context);
    double ctoPercent = getChannelFloat(t_context, 0);

    for(auto curCto : ctos)
    {
        auto cto = static_cast<CTOCapability*>(curCto);
        cto->setPercent(ctoPercent, t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
