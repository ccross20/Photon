#include "gobostate.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/capability/wheelshakecapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

GoboState::GoboState() : StateCapability(CapabilityType::Capability_WheelSlot)
{
    wheelOptions = QStringList{"Rotating Gobo Wheel", "Static Gobo Wheel","Animation Wheel","Color Wheel","Index"};
    setName("Gobo Wheel Slot");

    auto wheelInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Wheel","Which wheel to use","Color Wheel");
    wheelInfo.options = wheelOptions;
    addAvailableChannel(wheelInfo);

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Wheel Index","Which Wheel",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Gobo Slot","Which Slot",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Shake","How much to shake the gobo",0));
}


void GoboState::evaluate(const StateEvaluationContext &t_context) const
{

    int wheelType = getChannelInteger(t_context,0);
    int wheelIndex = getChannelInteger(t_context,1);
    int slotIndex = getChannelInteger(t_context,2);
    float shakeAmount = getChannelFloat(t_context,3);

    QString wheelName;

    if(wheelType < wheelOptions.length()-1)
    {
        auto wheel = t_context.fixture->findWheel(wheelOptions[wheelType]);
        if(!wheel)
            return;
        wheelName = wheel->name();
    }
    else
    {
        if(wheelIndex < 0 || wheelIndex >= t_context.fixture->wheels().length())
            return;
        wheelName = t_context.fixture->wheels()[wheelIndex]->name();
        wheelName = wheelName.toLower();
    }


    if(shakeAmount > 0)
    {
        auto allShakes = t_context.fixture->findCapability(CapabilityType::Capability_WheelShake);

        for(auto curSlot : allShakes)
        {
            auto wheelSlot = static_cast<WheelShakeCapability*>(curSlot);


            if(wheelSlot->wheelName().toLower() == wheelName)
            {
                if(wheelSlot->slotNumber() == slotIndex)
                {
                    wheelSlot->setSpeed(shakeAmount,t_context.dmxMatrix);
                    return;
                }


            }
        }
    }



    auto allSlots = t_context.fixture->findCapability(CapabilityType::Capability_WheelSlot);


    for(auto curSlot : allSlots)
    {
        auto wheelSlot = static_cast<WheelSlotCapability*>(curSlot);


        if(wheelSlot->wheelName().toLower() == wheelName)
        {
            if(wheelSlot->slotNumber() == slotIndex)
            {
                wheelSlot->selectSlot(t_context.dmxMatrix);
                return;
            }

        }
    }

}

} // namespace photon
