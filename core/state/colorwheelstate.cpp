#include <QColor>
#include "colorwheelstate.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

ColorWheelState::ColorWheelState() : StateCapability(CapabilityType::Capability_ColorWheelSlot)
{
    wheelOptions = QStringList{"Color Wheel", "Index"};


    setName("Color Wheel");

    auto info = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Mode","The state of the shutter",FixtureWheelSlot::Slot_Open);
    info.options = {"Open","Red","Cyan","Green","Blue","Yellow","Magenta"};
    addAvailableChannel(info);

    auto wheelInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Wheel","Which wheel to use","Color Wheel");
    wheelInfo.options = wheelOptions;
    addAvailableChannel(wheelInfo);

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Wheel Index","Which Color Wheel",1));
}

QVector3D colorToVector(const QColor &t_color)
{
    auto rgb = t_color.toRgb();

    return QVector3D{rgb.redF(), rgb.greenF(), rgb.blueF()};
}

void ColorWheelState::evaluate(const StateEvaluationContext &t_context) const
{
    auto allSlots = t_context.fixture->findCapability(Capability_WheelSlot);
    int color = getChannelInteger(t_context,0);
    int wheelType = getChannelInteger(t_context,1);
    int wheelIndex = getChannelInteger(t_context,2)-1;

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

    QVector<WheelSlotCapability*> colorSlots;

    for(auto curSlot : allSlots)
    {
        auto wheelSlot = static_cast<WheelSlotCapability*>(curSlot);


        if(wheelSlot->wheelName().toLower() == wheelName)
        {
            if(color == 0 && wheelSlot->wheelSlot()->type() == FixtureWheelSlot::Slot_Open)
            {
                wheelSlot->selectSlot(t_context.dmxMatrix);
                return;
            }

            else if(wheelSlot->wheelSlot()->type() == FixtureWheelSlot::Slot_Color)
            {
                colorSlots.append(wheelSlot);
            }

        }
    }

    QColor findColor;
    switch (color) {
    case 1:
        findColor = Qt::red;
        break;
    case 2:
        findColor = Qt::cyan;
        break;
    case 3:
        findColor = Qt::green;
        break;
    case 4:
        findColor = Qt::blue;
        break;
    case 5:
        findColor = Qt::yellow;
        break;
    case 6:
        findColor = Qt::magenta;
        break;
    default:
        return;
    }
    QVector3D findVector = colorToVector(findColor);


    float winningDistance = 10000.f;
    WheelSlotCapability *winner = nullptr;
    for(auto colorSlot : colorSlots)
    {
        float dist = findVector.distanceToPoint(colorToVector(static_cast<ColorSlot*>(colorSlot->wheelSlot())->color()));
        if(dist < winningDistance)
        {
            winningDistance = dist;
            winner = colorSlot;
        }
    }

    if(winner)
    {
        winner->selectSlot(t_context.dmxMatrix);
    }
}

} // namespace photon
