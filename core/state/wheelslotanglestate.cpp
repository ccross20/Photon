#include "wheelslotanglestate.h"
#include "fixture/capability/wheelslotrotationcapability.h"
#include "fixture/fixture.h"

namespace photon {

WheelSlotAngleState::WheelSlotAngleState() : StateCapability(CapabilityType::Capability_WheelSlotRotation)
{
    setName("Wheel Slot Angle");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Name","Name of the capability",""));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Angle","Angle of the ",0));

}

void WheelSlotAngleState::evaluate(const StateEvaluationContext &t_context) const
{

    float rotationAmount = getChannelFloat(t_context,1);
    QString name = getChannelString(t_context,0);

    if(!name.isEmpty())
    {
        auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_WheelSlotRotation, name);

        for(auto capability : allCapabilities)
        {
            auto wheelSlot = static_cast<WheelSlotRotationCapability*>(capability);

            if(wheelSlot->supportsAngle())
            {
                wheelSlot->setAngleDegrees(rotationAmount,t_context.dmxMatrix,1.0);
                return;
            }
        }
    }

}

} // namespace photon
