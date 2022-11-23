#include "fixturechannel.h"
#include "fixture.h"
#include "capability/shutterstrobecapability.h"
#include "capability/pancapability.h"
#include "capability/tiltcapability.h"

namespace photon {

class FixtureChannel::Impl
{
public:
    void addCapability(const QJsonObject &);
    QVector<FixtureCapability*> capabilities;
    QString name;
    int channelNumber;
    Fixture *fixture = nullptr;
    FixtureChannel *facade;

};

void FixtureChannel::Impl::addCapability(const QJsonObject &t_json)
{
    auto typeString = t_json.value("type").toString();
    FixtureCapability *capability = nullptr;

    if(typeString == "ShutterStrobe")
        capability = new ShutterStrobeCapability;
    if(typeString == "Pan")
        capability = new PanCapability;
    if(typeString == "Tilt")
        capability = new TiltCapability;

    if(capability)
    {
        capability->setChannel(facade);
        capability->readFromOpenFixtureJson(t_json);
        capabilities.append(capability);
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

void FixtureChannel::setChannelNumber(uchar t_channelNum)
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
}

} // namespace photon
