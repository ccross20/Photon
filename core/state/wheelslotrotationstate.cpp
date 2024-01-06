#include "wheelslotrotationstate.h"
#include "fixture/capability/wheelslotrotationcapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

WheelSlotRotationState::WheelSlotRotationState() : StateCapability(CapabilityType::Capability_WheelSlotRotation)
{
    setName("Wheel Slot Rotation");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Wheel Index","Which Wheel",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Rotation","How fast to rotate the gobo",0));

}

void WheelSlotRotationState::evaluate(const StateEvaluationContext &t_context) const
{

    int wheelIndex = getChannelInteger(t_context,0);
    float rotationAmount = getChannelFloat(t_context,1);

    if(wheelIndex < 0 || wheelIndex >= t_context.fixture->wheels().length())
        return;

    QString wheelName = t_context.fixture->wheels()[wheelIndex]->name();
    wheelName = wheelName.toLower();


    auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_WheelSlotRotation);

    for(auto capability : allCapabilities)
    {
        auto wheelSlot = static_cast<WheelSlotRotationCapability*>(capability);

        if(wheelSlot->wheel().toLower() == wheelName)
        {
            wheelSlot->setSpeed(rotationAmount,t_context.dmxMatrix,1.0);
            return;
        }
    }
}

} // namespace photon
