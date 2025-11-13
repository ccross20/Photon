#include "fixturevirtualchannel.h"
#include "fixturechannel.h"
#include "capability/colorcapability.h"

namespace photon {

class FixtureVirtualChannel::Impl
{
public:
    QVector<FixtureChannel*> channels;
    QVector<FixtureCapability*> capabilities;
    Fixture *fixture;
    QString name;
    CapabilityType type;
};

FixtureVirtualChannel::FixtureVirtualChannel(const QVector<FixtureChannel*> &t_channels, const QString &t_name):m_impl(new Impl)
{
    m_impl->channels = t_channels;
    m_impl->name = t_name;


    QVector<ColorIntensityCapability*> colors;
    for(auto channel : m_impl->channels)
    {
        for(auto cap : channel->capabilities())
        {
            auto intensity = dynamic_cast<ColorIntensityCapability *>(cap);
            if(intensity)
                colors.append(intensity);
        }
    }


    ColorCapability *colorCap = new ColorCapability(colors);


    m_impl->capabilities.append(colorCap);
    m_impl->type = Capability_Color;
}

FixtureVirtualChannel::~FixtureVirtualChannel()
{
    delete m_impl;
}

const QVector<FixtureChannel*> &FixtureVirtualChannel::channels() const
{
    return m_impl->channels;
}

bool FixtureVirtualChannel::isValid() const
{
    for(auto channel : m_impl->channels)
    {
        if(!channel->isValid())
            return false;
    }

    return !m_impl->channels.isEmpty();
}

QString FixtureVirtualChannel::name() const
{
    return m_impl->name;
}

CapabilityType FixtureVirtualChannel::capabilityType() const
{
    return m_impl->type;
}

const QVector<FixtureCapability*> &FixtureVirtualChannel::capabilities() const
{
    return m_impl->capabilities;
}

Fixture *FixtureVirtualChannel::fixture() const
{
    return m_impl->fixture;
}

void FixtureVirtualChannel::setFixture(Fixture *t_fixture)
{
    m_impl->fixture = t_fixture;
}

} // namespace photon
