#include "wheelslotrotationstate.h"
#include "fixture/capability/wheelslotrotationcapability.h"
#include "fixture/fixture.h"

namespace photon {

WheelSlotRotationState::WheelSlotRotationState() : StateCapability(CapabilityType::Capability_WheelSlotRotation)
{
    setName("Wheel Slot Rotation");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Name","Name of the capability",""));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Speed","How fast to rotate the gobo",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Use Angle","Use angle instead of speed",false));

}

void WheelSlotRotationState::evaluate(const StateEvaluationContext &t_context) const
{

    float rotationAmount = getChannelFloat(t_context,1);

    bool useAngle = getChannelBool(t_context,2);
    QString name = getChannelString(t_context,0);

    if(!name.isEmpty())
    {
        auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_WheelSlotRotation, name);

        for(auto capability : allCapabilities)
        {
            auto wheelSlot = static_cast<WheelSlotRotationCapability*>(capability);

            if(wheelSlot->supportsAngle() && useAngle)
            {
                wheelSlot->setAngleDegrees(rotationAmount,t_context.dmxMatrix,1.0);
                return;
            }
            else
            {
                if(rotationAmount > 0 && wheelSlot->maxSpeed() > 0)
                {
                    wheelSlot->setSpeed(rotationAmount,t_context.dmxMatrix,1.0);
                    return;
                }
                else if(rotationAmount < 0 && wheelSlot->maxSpeed() < 0)
                {
                    wheelSlot->setSpeed(rotationAmount,t_context.dmxMatrix,1.0);
                    return;
                }
            }
        }
    }

}

} // namespace photon
