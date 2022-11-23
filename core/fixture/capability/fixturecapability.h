#ifndef PHOTON_FIXTURECAPABILITY_H
#define PHOTON_FIXTURECAPABILITY_H
#include "photon-global.h"

namespace photon {

struct DMXRange{
    DMXRange(uchar start = 0, uchar end = 255):start(start),end(end){}
    uchar start = 0;
    uchar end = 255;
};

enum CapabilityType{
    Capability_Unknown,
    Capability_Pan,
    Capability_Tilt
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
    Speed_Fast,
    Speed_Slow,
    Speed_Stop,
    Speed_SlowReverse,
    Speed_FastReverse
};

enum RotationSpeedEnum{
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

    CapabilityType type() const;

    virtual void readFromOpenFixtureJson(const QJsonObject &);

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

    static FixtureUnit speed(const QString &value, double *valueWithoutUnits = nullptr);
    static FixtureUnit rotationSpeed(const QString &value, double *valueWithoutUnits = nullptr);
    static FixtureUnit rotationAngle(const QString &value, double *valueWithoutUnits = nullptr);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECAPABILITY_H
