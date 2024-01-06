#include "fixturechannel_p.h"
#include "fixture.h"
#include "capability/shutterstrobecapability.h"
#include "capability/anglecapability.h"
#include "capability/dimmercapability.h"
#include "capability/wheelslotcapability.h"
#include "capability/wheelshakecapability.h"
#include "capability/colorintensitycapability.h"
#include "capability/wheelrotationcapability.h"
#include "capability/wheelslotrotationcapability.h"

namespace photon {

void FixtureChannel::Impl::addCapability(const QJsonObject &t_json)
{
    auto typeString = t_json.value("type").toString().toLower();
    FixtureCapability *capability = nullptr;

    if(typeString == "shutterstrobe")
        capability = new ShutterStrobeCapability;
    else if(typeString == "pan")
        capability = new AngleCapability(Capability_Pan);
    else if(typeString == "tilt")
        capability = new AngleCapability(Capability_Tilt);
    else if(typeString == "zoom")
        capability = new AngleCapability(Capability_Zoom);
    else if(typeString == "focus")
        capability = new AngleCapability(Capability_Focus);
    else if(typeString == "intensity")
        capability = new DimmerCapability(Capability_Dimmer);
    else if(typeString == "wheelslot")
        capability = new WheelSlotCapability();
    else if(typeString == "wheelshake")
        capability = new WheelShakeCapability();
    else if(typeString == "wheelrotation")
        capability = new WheelRotationCapability();
    else if(typeString == "wheelslotrotation")
        capability = new WheelSlotRotationCapability();
    else if(typeString == "colorintensity")
    {
        auto colorString = t_json.value("color").toString().toLower();
        if(colorString == "cyan")
            capability = new ColorIntensityCapability(Capability_Cyan);
        else if(colorString == "magenta")
            capability = new ColorIntensityCapability(Capability_Magenta);
        else if(colorString == "yellow")
            capability = new ColorIntensityCapability(Capability_Yellow);
    }

    if(capability)
    {
        capability->setChannel(facade);
        capability->readFromOpenFixtureJson(t_json);
        capabilities.append(capability);
        type = capability->type();
    }
}

FixtureChannel::FixtureChannel(const QString &t_name, int t_channelNumber):m_impl(new Impl)
{
    m_impl->facade = this;
    m_impl->name = t_name;
    m_impl->channelNumber = t_channelNumber;
}

FixtureChannel::~FixtureChannel()
{
    delete m_impl;
}

bool FixtureChannel::isValid() const
{
    return m_impl->channelNumber >= 0;
}

CapabilityType FixtureChannel::capabilityType() const
{
    return m_impl->type;
}

const QVector<FixtureChannel*> &FixtureChannel::fineChannels() const
{
    return m_impl->fineChannels;
}

QString FixtureChannel::name() const
{
    return m_impl->name;
}

int FixtureChannel::channelNumber() const
{
    return m_impl->channelNumber;
}

int FixtureChannel::universalChannelNumber() const
{
    if(m_impl->channelNumber >= 0)
        return m_impl->fixture->dmxOffset() + m_impl->channelNumber;
    return -1;
}

int FixtureChannel::universe() const
{
    return fixture()->universe();
}

void FixtureChannel::setChannelNumber(int t_channelNum)
{
    m_impl->channelNumber = t_channelNum;
}

Fixture *FixtureChannel::fixture() const
{
    return m_impl->fixture;
}

void FixtureChannel::setFixture(Fixture *t_fixture)
{
    m_impl->fixture = t_fixture;
}

const QVector<FixtureCapability*> &FixtureChannel::capabilities() const
{
    return m_impl->capabilities;
}

void FixtureChannel::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    if(t_json.contains("capability"))
    {
        m_impl->addCapability(t_json.value("capability").toObject());
    }
    if(t_json.contains("capabilities"))
    {
        auto capabilityArray = t_json.value("capabilities").toArray();
        for(auto capability : capabilityArray)
        {
            m_impl->addCapability(capability.toObject());
        }
    }
    if(t_json.contains("fineChannelAliases"))
    {
        auto fineArray = t_json.value("fineChannelAliases").toArray();
        for(auto alias : fineArray)
        {
            m_impl->fineChannelsAliases.append(alias.toString().toLatin1());
        }
    }
}

} // namespace photon
