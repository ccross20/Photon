#include <QColor>
#include "colorwheelstate.h"
#include "fixture/capability/wheelslotcapability.h"
#include "fixture/fixturewheel.h"
#include "fixture/fixture.h"

namespace photon {

ColorWheelState::ColorWheelState() : StateCapability(CapabilityType::Capability_ColorWheelSlot)
{

    setName("Color Wheel");


    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Name","Name of the wheel",""));

    auto info = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Color","The state of the shutter",FixtureWheelSlot::Slot_Open);
    info.options = {"Open","Red","Cyan","Green","Blue","Yellow","Magenta"};
    addAvailableChannel(info);

}

QVector3D colorToVector(const QColor &t_color)
{
    auto rgb = t_color.toRgb();

    return QVector3D{rgb.redF(), rgb.greenF(), rgb.blueF()};
}

void ColorWheelState::evaluate(const StateEvaluationContext &t_context) const
{
    auto allSlots = t_context.fixture->findCapability(Capability_WheelSlot);


    QString name = getChannelString(t_context,0);
    int color = getChannelInteger(t_context,1);


    QVector<WheelSlotCapability*> colorSlots;
    if(!name.isEmpty())
    {
        auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_WheelSlot, name);

        for(auto capability : allCapabilities)
        {
            auto wheelSlot = static_cast<WheelSlotCapability*>(capability);

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
    else
    {
        return;
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
