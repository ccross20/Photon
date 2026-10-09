#include <cmath>
#include <QColor>
#include <QPointF>
#include "laserstates.h"
#include "fixture/capability/lasercapability.h"

namespace photon {

using Function = LaserCapability::Function;

static const LaserCapability *laserChannel(const StateEvaluationContext &t_context, Function t_function)
{
    return LaserCapability::find(t_context.fixture, t_function);
}

LaserContentState::LaserContentState() : StateCapability(Capability_LaserContent)
{
    setName("Content");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Page", "Content page (0 = no output)", 1));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeInteger, "Cue", "Cue on the page (0 = no output)", 1));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Cue Speed", "Playback speed (1 = normal)", 1.0, 0.0, 2.55));
}

void LaserContentState::evaluate(const StateEvaluationContext &t_context) const
{
    if(auto *page = laserChannel(t_context, Function::Function_Page))
        page->setRaw(getChannelInteger(t_context, 0), t_context.dmxMatrix, t_context.strength);
    if(auto *cue = laserChannel(t_context, Function::Function_Cue))
        cue->setRaw(getChannelInteger(t_context, 1), t_context.dmxMatrix, t_context.strength);
    if(auto *speed = laserChannel(t_context, Function::Function_CueSpeed))
        speed->setRaw(int(std::lround(getChannelFloat(t_context, 2) * 100.0)), t_context.dmxMatrix, t_context.strength);
}

LaserSizeState::LaserSizeState() : StateCapability(Capability_LaserSize)
{
    setName("Size");
    // Absolute on the FB4: 1 = full size, 0 = collapsed, negative = mirrored.
    // The content is Zoom x Size X wide and Zoom x Size Y tall.
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Zoom", "-1 to 1, 1 = full size, 0 = collapsed, negative = mirrored", 1.0, -1.0, 1.0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Size X", "-1 to 1, 1 = full width, 0 = collapsed, negative = mirrored", 1.0, -1.0, 1.0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Size Y", "-1 to 1, 1 = full height, 0 = collapsed, negative = mirrored", 1.0, -1.0, 1.0));
}

void LaserSizeState::evaluate(const StateEvaluationContext &t_context) const
{
    if(auto *zoom = laserChannel(t_context, Function::Function_Zoom))
        zoom->setCentered(getChannelFloat(t_context, 0), t_context.dmxMatrix, t_context.strength);
    if(auto *sizeX = laserChannel(t_context, Function::Function_SizeX))
        sizeX->setCentered(getChannelFloat(t_context, 1), t_context.dmxMatrix, t_context.strength);
    if(auto *sizeY = laserChannel(t_context, Function::Function_SizeY))
        sizeY->setCentered(getChannelFloat(t_context, 2), t_context.dmxMatrix, t_context.strength);
}

LaserPositionState::LaserPositionState() : StateCapability(Capability_LaserPosition)
{
    setName("Position");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypePoint, "Position", "X and Y, -1 to 1, (0, 0) = center",
                                    QPointF(0.0, 0.0), -1.0, 1.0));
}

void LaserPositionState::evaluate(const StateEvaluationContext &t_context) const
{
    const QPointF position = getChannelPoint(t_context, 0);
    if(auto *x = laserChannel(t_context, Function::Function_PositionX))
        x->setCentered(position.x(), t_context.dmxMatrix, t_context.strength);
    if(auto *y = laserChannel(t_context, Function::Function_PositionY))
        y->setCentered(position.y(), t_context.dmxMatrix, t_context.strength);
}

LaserRotationState::LaserRotationState() : StateCapability(Capability_LaserRotation)
{
    setName("Rotation");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Angle", "Degrees", 0.0, 0.0, 360.0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Speed", "-1 to 1 (-60 to 60 rpm), 0 = hold angle", 0.0, -1.0, 1.0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Use Cue Rotation", "Ignore Speed and keep the cue's own rotation", false));
}

void LaserRotationState::evaluate(const StateEvaluationContext &t_context) const
{
    if(auto *angle = laserChannel(t_context, Function::Function_Angle))
    {
        double degrees = std::fmod(getChannelFloat(t_context, 0), 360.0);
        if(degrees < 0.0)
            degrees += 360.0;
        angle->setPercent(degrees / 360.0, t_context.dmxMatrix, t_context.strength);
    }

    auto *rotation = laserChannel(t_context, Function::Function_Rotation);
    if(!rotation)
        return;

    if(getChannelBool(t_context, 2))
    {
        rotation->setRaw(0, t_context.dmxMatrix, t_context.strength);
        return;
    }

    // 0 is "original", so the reverse range starts at 1; 32768 holds the angle.
    const double speed = std::clamp(double(getChannelFloat(t_context, 1)), -1.0, 1.0);
    int word = 32768;
    if(speed < 0.0)
        word = int(std::lround(32767.0 + 32766.0 * speed));
    else if(speed > 0.0)
        word = int(std::lround(32768.0 + 32767.0 * speed));
    if(!rotation->is16Bit())
        word >>= 8;
    rotation->setRaw(word, t_context.dmxMatrix, t_context.strength);
}

LaserColorState::LaserColorState() : StateCapability(Capability_LaserColor)
{
    setName("Color");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeColor, "Color", "Color mixed into the cue", QColor(Qt::white)));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Mix", "0 = cue's own color, 1 = this color", 1.0, 0.0, 1.0));
}

void LaserColorState::evaluate(const StateEvaluationContext &t_context) const
{
    const QColor color = getChannelColor(t_context, 0);
    if(auto *red = laserChannel(t_context, Function::Function_Red))
        red->setPercent(color.redF(), t_context.dmxMatrix, t_context.strength);
    if(auto *green = laserChannel(t_context, Function::Function_Green))
        green->setPercent(color.greenF(), t_context.dmxMatrix, t_context.strength);
    if(auto *blue = laserChannel(t_context, Function::Function_Blue))
        blue->setPercent(color.blueF(), t_context.dmxMatrix, t_context.strength);
    if(auto *mix = laserChannel(t_context, Function::Function_ColorMix))
        mix->setPercent(getChannelFloat(t_context, 1), t_context.dmxMatrix, t_context.strength);
}

LaserScanRateState::LaserScanRateState() : StateCapability(Capability_LaserScanRate)
{
    setName("Scan Rate");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Scan Rate", "0 = 5K, 1 = cue's own scan rate", 1.0, 0.0, 1.0));
}

void LaserScanRateState::evaluate(const StateEvaluationContext &t_context) const
{
    if(auto *scanRate = laserChannel(t_context, Function::Function_ScanRate))
        scanRate->setPercent(getChannelFloat(t_context, 0), t_context.dmxMatrix, t_context.strength);
}

LaserVisiblePointsState::LaserVisiblePointsState() : StateCapability(Capability_LaserVisiblePoints)
{
    setName("Visible Points");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Start", "Points cut from the start of the frame", 0.0, 0.0, 1.0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "End", "Points cut from the end of the frame", 0.0, 0.0, 1.0));
}

void LaserVisiblePointsState::evaluate(const StateEvaluationContext &t_context) const
{
    if(auto *start = laserChannel(t_context, Function::Function_VisibleStart))
        start->setPercent(getChannelFloat(t_context, 0), t_context.dmxMatrix, t_context.strength);
    if(auto *end = laserChannel(t_context, Function::Function_VisibleEnd))
        end->setPercent(getChannelFloat(t_context, 1), t_context.dmxMatrix, t_context.strength);
}

LaserStrobeState::LaserStrobeState() : StateCapability(Capability_LaserStrobe)
{
    setName("Strobe");
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Rate", "Hz, 0 = off", 0.0, 0.0, 20.0));
}

void LaserStrobeState::evaluate(const StateEvaluationContext &t_context) const
{
    auto *strobe = laserChannel(t_context, Function::Function_Strobe);
    if(!strobe)
        return;

    const double hertz = getChannelFloat(t_context, 0);
    int word = 0;
    if(hertz > 0.0)
        word = int(std::lround(1.0 + (std::clamp(hertz, 1.0, 20.0) - 1.0) / 19.0 * 254.0));
    strobe->setRaw(word, t_context.dmxMatrix, t_context.strength);
}

} // namespace photon
