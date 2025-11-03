#include "wheelrotationstate.h"
#include "fixture/capability/wheelrotationcapability.h"
#include "fixture/fixture.h"

namespace photon {

WheelRotationState::WheelRotationState() : StateCapability(CapabilityType::Capability_WheelRotation)
{
    setName("Wheel Rotation");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Name","Name of the capability",""));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Speed","How much to shake the gobo",0));

}


void WheelRotationState::evaluate(const StateEvaluationContext &t_context) const
{

    float rotationAmount = getChannelFloat(t_context,1);
    QString name = getChannelString(t_context,0);

    if(!name.isEmpty())
    {
        auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_WheelRotation, name);

        for(auto capability : allCapabilities)
        {
            auto wheelSlot = static_cast<WheelRotationCapability*>(capability);


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

} // namespace photon
