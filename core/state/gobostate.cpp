#include <QJsonArray>
#include <QJsonObject>
#include "gobostate.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/capability/wheelshakecapability.h"
#include "fixture/fixture.h"

namespace photon {

namespace {

enum GoboChannel { WheelChannel, SlotChannel, ShakeChannel, RotateModeChannel };

} // namespace

GoboState::GoboState() : StateCapability(CapabilityType::Capability_WheelSlot)
{
    rotateModeOptions = QStringList{"Any", "Index", "Continuous"};
    setName("Gobo Wheel Slot");

    // A text channel: the Fixture State editor offers the wheel names of the
    // fixtures it applies to (see FixtureStateEditor).
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Wheel",
                                    "Wheel name (empty = the fixture's first gobo wheel)", QString()));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Slot", "Slot number, starting at 1", 1));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Shake", "How much to shake the gobo", 0, 0.0, 1.0));

    // Some fixtures expose the same slot twice, once indexed and once
    // continuous; without this the first match won regardless of which one
    // was wanted. The Set Fixture Slot node matches on rotateMode the same way.
    auto rotateModeInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Rotate Mode", "Only match slots of this rotate mode", 0);
    rotateModeInfo.options = rotateModeOptions;
    addAvailableChannel(rotateModeInfo);
}

void GoboState::evaluate(const StateEvaluationContext &t_context) const
{
    if(!t_context.fixture)
        return;

    QString wheelName = t_context.channelValues.value(channelId(WheelChannel)).toString().trimmed();
    if(wheelName.isEmpty())
        wheelName = t_context.fixture->resolveWheelName("gobo wheel");
    if(wheelName.isEmpty())
        return;

    const int slotNumber = getChannelInteger(t_context, SlotChannel);
    const float shakeAmount = getChannelFloat(t_context, ShakeChannel);
    const int rotateModeChoice = getChannelInteger(t_context, RotateModeChannel);

    // Index 0 is "Any" - match whatever the fixture offers.
    const bool filterRotateMode = rotateModeChoice > 0;
    const WheelSlotCapability::RotateMode wantedRotateMode = rotateModeChoice == 1
        ? WheelSlotCapability::RotateMode_Index
        : WheelSlotCapability::RotateMode_Continuous;

    if(shakeAmount > 0)
    {
        for(auto *capability : t_context.fixture->findCapability(CapabilityType::Capability_WheelShake))
        {
            auto *shake = static_cast<WheelShakeCapability*>(capability);
            if(shake->wheelName().compare(wheelName, Qt::CaseInsensitive) == 0
               && shake->slotNumber() == slotNumber)
            {
                shake->setSpeed(shakeAmount, t_context.dmxMatrix, t_context.strength);
                return;
            }
        }
    }

    for(auto *capability : t_context.fixture->findCapability(CapabilityType::Capability_WheelSlot))
    {
        auto *wheelSlot = static_cast<WheelSlotCapability*>(capability);
        if(wheelSlot->wheelName().compare(wheelName, Qt::CaseInsensitive) != 0)
            continue;
        if(filterRotateMode && wheelSlot->rotateMode() != wantedRotateMode)
            continue;
        if(wheelSlot->slotNumber() == slotNumber)
        {
            wheelSlot->selectSlot(t_context.dmxMatrix);
            return;
        }
    }
}

void GoboState::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    // Saves from before the Wheel became a name held five values: wheel type
    // (an index into a fixed list of wheel names fixtures rarely use), wheel
    // index, slot, shake, rotate mode. Values load by position, so move them
    // to the current layout. The old wheel choice can't be carried over, so
    // it becomes empty - the fixture's first gobo wheel.
    QJsonObject json = t_json;
    const QJsonArray values = t_json.value("values").toArray();
    if(values.size() == 5)
        json.insert("values", QJsonArray{QString(), values[2], values[3], values[4]});

    StateCapability::readFromJson(json, t_context);
}

} // namespace photon
