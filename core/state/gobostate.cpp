#include "gobostate.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/capability/wheelshakecapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

GoboState::GoboState() : StateCapability(CapabilityType::Capability_WheelSlot)
{
    wheelOptions = QStringList{"Rotating Gobo Wheel", "Static Gobo Wheel","Animation Wheel","Color Wheel","Index"};
    rotateModeOptions = QStringList{"Any", "Index", "Continuous"};
    setName("Gobo Wheel Slot");

    auto wheelInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Wheel","Which wheel to use","Color Wheel");
    wheelInfo.options = wheelOptions;
    addAvailableChannel(wheelInfo);

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Wheel Index","Which Wheel",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Gobo Slot","Which Slot",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Shake","How much to shake the gobo",0));

    // APPENDED, never inserted: StateCapability::readFromJson restores values by
    // index, so putting a channel anywhere but the end would silently shift every
    // saved value. A project saved before this channel existed simply keeps the
    // "Any" default, which is how the state behaved back then.
    //
    // Some fixtures expose the same slot twice, once indexed and once continuous;
    // without this the first match won regardless of which one was wanted. The
    // Set Fixture Slot node has always matched on rotateMode - this is the state
    // catching up with it.
    auto rotateModeInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Rotate Mode","Only match slots of this rotate mode",0);
    rotateModeInfo.options = rotateModeOptions;
    addAvailableChannel(rotateModeInfo);
}


void GoboState::evaluate(const StateEvaluationContext &t_context) const
{

    int wheelType = getChannelInteger(t_context,0);
    int wheelIndex = getChannelInteger(t_context,1);
    int slotIndex = getChannelInteger(t_context,2);
    float shakeAmount = getChannelFloat(t_context,3);
    int rotateModeChoice = getChannelInteger(t_context,4);

    // Index 0 is "Any" - match whatever the fixture offers, which is what this
    // state did before the channel existed.
    const bool filterRotateMode = rotateModeChoice > 0;
    const WheelSlotCapability::RotateMode wantedRotateMode = rotateModeChoice == 1
        ? WheelSlotCapability::RotateMode_Index
        : WheelSlotCapability::RotateMode_Continuous;

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
                    wheelSlot->setSpeed(shakeAmount,t_context.dmxMatrix,t_context.strength);
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
            if(filterRotateMode && wheelSlot->rotateMode() != wantedRotateMode)
                continue;

            if(wheelSlot->slotNumber() == slotIndex)
            {
                wheelSlot->selectSlot(t_context.dmxMatrix);
                return;
            }

        }
    }

}

} // namespace photon
