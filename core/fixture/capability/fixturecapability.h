#ifndef PHOTON_FIXTURECAPABILITY_H
#define PHOTON_FIXTURECAPABILITY_H
#include "photon-global.h"

#include "sequence/channel.h"

namespace photon {

struct DMXRange{
    DMXRange(uchar start = 0, uchar end = 255):start(start),end(end){}
    uchar start = 0;
    uchar end = 255;

    bool contains(uchar t_value) const
    {
        return t_value >= start && t_value <= end;
    }

    uchar middle() const
    {
        return start + ((end - start) / 2);
    }

    uchar fromPercent(double t_value)
    {
        return start + std::round((end - start) * t_value);
    }

    double toPercent(uchar t_value)
    {
        double s = t_value - start;
        double r = end - start;
        return s / r;
    }
};

enum CapabilityType{
    Capability_Unknown,
    Capability_Pan,
    Capability_Tilt,
    Capability_TiltAngle,
    Capability_TiltAngleCentered,
    Capability_Focus,
    Capability_Zoom,
    Capability_Dimmer,
    Capability_Strobe,
    Capability_Cyan,
    Capability_Magenta,
    Capability_Yellow,
    Capability_Red,
    Capability_Green,
    Capability_Blue,
    Capability_Lime,
    Capability_White,
    Capability_Amber,
    Capability_Indigo,
    Capability_UV,
    Capability_CTO,
    Capability_Color,
    Capability_NoFunction,
    Capability_WheelSlot,
    Capability_ColorWheelSlot,
    Capability_WheelShake,
    Capability_WheelSlotRotation,
    Capability_WheelRotation
};

enum FixtureUnit{
    Unit_Unknown,
    Unit_Number,
    Unit_NonNegativeNumber,
    Unit_PositiveInteger,
    Unit_DMXValue,
    Unit_DMXPercent,
    Unit_Percent,
    Unit_Hertz,
    Unit_BeatsPerMinute,
    Unit_RoundsPerMinute,
    Unit_Seconds,
    Unit_Milliseconds,
    Unit_Meters,
    Unit_Lumens,
    Unit_Kelvin,
    Unit_VolumePerMinute,
    Unit_Degrees,
    Unit_Enum
};

enum SpeedEnum{
    Speed_Unknown,
    Speed_Fast,
    Speed_Slow,
    Speed_Stop,
    Speed_SlowReverse,
    Speed_FastReverse
};

enum RotationSpeedEnum{
    RotationSpeed_Unknown,
    RotationSpeed_FastCW,
    RotationSpeed_SlowCW,
    RotationSpeed_Stop,
    RotationSpeed_SlowCCW,
    RotationSpeed_FastCCW
};

enum TimeEnum{
    Time_Instant,
    Time_Short,
    Time_Long
};

class PHOTONCORE_EXPORT FixtureCapability
{
public:
    FixtureCapability(DMXRange range, CapabilityType capability = Capability_Unknown);
    virtual ~FixtureCapability();

    DMXRange range() const;

    Fixture *fixture() const;
    FixtureChannel *channel() const;
    void setChannel(FixtureChannel *);
    FixtureChannel *fineChannel() const;
    bool isValid(const DMXMatrix &t_matrix) const;

    CapabilityType type() const;

    virtual void readFromOpenFixtureJson(const QJsonObject &);

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

    static FixtureUnit speed(const QString &value, double *valueWithoutUnits = nullptr);
    static FixtureUnit rotationSpeed(const QString &value, double *valueWithoutUnits = nullptr);
    static FixtureUnit rotationAngle(const QString &value, double *valueWithoutUnits = nullptr);
    static FixtureUnit distance(const QString &value, double *valueWithoutUnits = nullptr);
    static bool constantPercent(const QString &value, double *valueWithoutUnits = nullptr);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECAPABILITY_H
