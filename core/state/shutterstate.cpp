#include "shutterstate.h"
#include "fixture/capability/shutterstrobecapability.h"

namespace photon {

ShutterState::ShutterState() : StateCapability(CapabilityType::Capability_Strobe)
{
    setName("Shutter / Strobe");

    auto info = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Mode","The state of the shutter",ShutterStrobeCapability::Shutter_Open);
    info.options = {"Open","Closed","Strobe","Pulse","Ramp Up","Ramp Down","Ramp Up Down","Lightning","Spikes","Burst"};
    addAvailableChannel(info);
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Speed","How quickly the strobe fires",.5));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Random","The strobe fires at a random rate",false));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Sound Activated","Fire the shutter in sync with the sound",false));
}

void ShutterState::evaluate(const StateEvaluationContext &t_context) const
{
    auto strobes = getFixtureCapabilities(t_context);
    double speed = getChannelFloat(t_context,1);
    bool isRandom = getChannelBool(t_context,2) > 0;
    bool isSoundActivated = getChannelBool(t_context,3) > 0;
    ShutterStrobeCapability::ShutterEffect mode = static_cast<ShutterStrobeCapability::ShutterEffect>(getChannelInteger(t_context,0));

    for(auto curStrobe : strobes)
    {
        auto strobe = static_cast<ShutterStrobeCapability*>(curStrobe);

        if(strobe->shutterEffect() == mode &&
            strobe->hasRandomCapability() == isRandom &&
            strobe->hasSoundCapability() == isSoundActivated)
        {
            strobe->setSpeedPercent(speed, t_context.dmxMatrix);
            return;
        }
    }
}

} // namespace photon
