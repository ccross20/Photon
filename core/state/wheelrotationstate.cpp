#include "wheelrotationstate.h"
#include "fixture/capability/wheelrotationcapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

WheelRotationState::WheelRotationState() : StateCapability(CapabilityType::Capability_WheelRotation)
{
    setName("Wheel Rotation");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Wheel Index","Which Wheel",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Rotation","How much to shake the gobo",0));

}


void WheelRotationState::evaluate(const StateEvaluationContext &t_context) const
{

    int wheelIndex = getChannelInteger(t_context,0);
    float rotationAmount = getChannelFloat(t_context,1);

    if(wheelIndex < 0 || wheelIndex >= t_context.fixture->wheels().length())
        return;

    QString wheelName = t_context.fixture->wheels()[wheelIndex]->name();
    wheelName = wheelName.toLower();


    auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_WheelRotation);

    for(auto capability : allCapabilities)
    {
        auto wheelSlot = static_cast<WheelRotationCapability*>(capability);

        if(wheelSlot->wheelName().toLower() == wheelName)
        {
            wheelSlot->setSpeed(rotationAmount,t_context.dmxMatrix,1.0);
            return;
        }
    }
}

} // namespace photon
