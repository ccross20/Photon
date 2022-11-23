#include "shutterstrobecapability.h"

namespace photon {

class ShutterStrobeCapability::Impl
{
public:
    ShutterEffect effect = Shutter_Open;
    double speed = 0.0;
    double speedStart = 0.0;
    double speedEnd = 0.0;
    double duration = 0.0;
    double durationStart = 0.0;
    double durationEnd = 0.0;
    bool hasRandom = false;
    bool hasSound = false;
};

ShutterStrobeCapability::ShutterStrobeCapability(DMXRange range) : FixtureCapability(range),m_impl(new Impl)
{

}

ShutterStrobeCapability::~ShutterStrobeCapability()
{
    delete m_impl;
}

ShutterStrobeCapability::ShutterEffect ShutterStrobeCapability::shutterEffect() const
{
    return m_impl->effect;
}

void ShutterStrobeCapability::setSpeed(double value)
{
    m_impl->speed = value;
}

bool ShutterStrobeCapability::hasSoundCapability() const
{
    return m_impl->hasSound;
}

bool ShutterStrobeCapability::hasRandomCapability() const
{
    return m_impl->hasRandom;
}

void ShutterStrobeCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

    auto effectString = t_json.value("shutterEffect").toString("Open");
    if(effectString == "Open")
        m_impl->effect = Shutter_Open;
    else if(effectString == "Closed")
        m_impl->effect = Shutter_Closed;
    else if(effectString == "Strobe")
        m_impl->effect = Shutter_Strobe;
    else if(effectString == "Pulse")
        m_impl->effect = Shutter_Pulse;
    else if(effectString == "RampUp")
        m_impl->effect = Shutter_RampUp;
    else if(effectString == "RampDown")
        m_impl->effect = Shutter_RampDown;
    else if(effectString == "RampUpDown")
        m_impl->effect = Shutter_RampUpDown;
    else if(effectString == "Lightning")
        m_impl->effect = Shutter_Lightning;
    else if(effectString == "Spikes")
        m_impl->effect = Shutter_Spikes;
    else if(effectString == "Burst")
        m_impl->effect = Shutter_Burst;

    m_impl->hasSound = t_json.value("soundControlled").toBool(false);
    m_impl->hasRandom = t_json.value("randomTiming").toBool(false);

    m_impl->speed = t_json.value("speed").toDouble(0.0);
}


} // namespace photon
