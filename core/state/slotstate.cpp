#include "slotstate.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

SlotState::SlotState() : StateCapability(CapabilityType::Capability_WheelSlot)
{
    wheelOptions = QStringList{"Rotating Gobo Wheel", "Static Gobo Wheel","Animation Wheel","Color Wheel","Index"};


    setName("Gobo Slot");


    auto wheelInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Wheel","Which wheel to use","Color Wheel");
    wheelInfo.options = wheelOptions;
    addAvailableChannel(wheelInfo);

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Wheel Index","Which Color Wheel",1));

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Slot Index","Which Slot",1));
}

void SlotState::evaluate(const StateEvaluationContext &t_context) const
{
    auto allSlots = t_context.fixture->findCapability(Capability_WheelSlot);
    int wheelType = getChannelInteger(t_context,0);
    int wheelIndex = getChannelInteger(t_context,1)-1;
    int slotIndex = getChannelInteger(t_context,1)-1;

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
    }

    wheelName = t_context.fixture->wheels()[wheelIndex]->name();
    wheelName = wheelName.toLower();

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
