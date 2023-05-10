#include "shutterstrobecapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class ShutterStrobeCapability::Impl
{
public:
    ShutterEffect effect = Shutter_Open;
    double speed = 0.0;
    double speedStart = 0.0;
    double speedEnd = 1.0;
    double duration = 0.0;
    double durationStart = 0.0;
    double durationEnd = 0.0;
    bool hasRandom = false;
    bool hasSound = false;
};

ShutterStrobeCapability::ShutterStrobeCapability(DMXRange range) : FixtureCapability(range, Capability_Strobe),m_impl(new Impl)
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

void ShutterStrobeCapability::setSpeedPercent(double value, DMXMatrix &t_matrix)
{
    t_matrix.setValue(fixture()->universe()-1,channel()->universalChannelNumber(),range().fromPercent(value));
}

double ShutterStrobeCapability::getSpeedPercent(const DMXMatrix &t_matrix)
{
    double speedRange = m_impl->speedEnd - m_impl->speedStart;
    return m_impl->speedStart + (range().toPercent(t_matrix.value(fixture()->universe()-1,channel()->universalChannelNumber())) * speedRange);
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

    if(t_json.value("speed").isDouble())
    {
        m_impl->speed = t_json.value("speed").toDouble(0.0);
    }
    else
    {
        auto speedString = t_json.value("speed").toString();
        double value = 0;
        if(constantPercent(speedString, &value))
            m_impl->speed = value;
    }

    if(t_json.contains("speedStart"))
    {
        double value = 0;
        if(constantPercent(t_json.value("speedStart").toString(), &value))
            m_impl->speedStart = value;
    }

    if(t_json.contains("speedEnd"))
    {
        double value = 0;
        if(constantPercent(t_json.value("speedEnd").toString(), &value))
            m_impl->speedEnd = value;
    }



}


} // namespace photon
