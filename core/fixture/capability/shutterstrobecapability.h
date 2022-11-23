#ifndef PHOTON_SHUTTERSTROBECAPABILITY_H
#define PHOTON_SHUTTERSTROBECAPABILITY_H
#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT ShutterStrobeCapability : public FixtureCapability
{
public:
    enum ShutterEffect{
        Shutter_Open,
        Shutter_Closed,
        Shutter_Strobe,
        Shutter_Pulse,
        Shutter_RampUp,
        Shutter_RampDown,
        Shutter_RampUpDown,
        Shutter_Lightning,
        Shutter_Spikes,
        Shutter_Burst
    };


    ShutterStrobeCapability(DMXRange range = DMXRange{});
    ~ShutterStrobeCapability();

    ShutterEffect shutterEffect() const;

    void setSpeed(double value);

    bool hasSoundCapability() const;
    bool hasRandomCapability() const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SHUTTERSTROBECAPABILITY_H
