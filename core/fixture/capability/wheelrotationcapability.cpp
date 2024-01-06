#include "wheelrotationcapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixturechannel.h"

namespace photon {


class WheelRotationCapability::Impl
{
public:
    QString wheelName;
    double startSpeed = 0;
    double stopSpeed = 0;
};


WheelRotationCapability::WheelRotationCapability() : FixtureCapability(DMXRange(),Capability_WheelRotation),m_impl(new Impl)
{

}

WheelRotationCapability::~WheelRotationCapability()
{
    delete m_impl;
}

double WheelRotationCapability::speedStart() const
{
    return m_impl->startSpeed;
}

double WheelRotationCapability::speedEnd() const
{
    return m_impl->stopSpeed;
}

QString WheelRotationCapability::wheelName() const
{
    return m_impl->wheelName;
}

void WheelRotationCapability::setSpeed(double value, DMXMatrix &t_matrix, double t_blend)
{
    if(value < 0)
        value *= -1.0;

    if(m_impl->stopSpeed > m_impl->startSpeed)
        value = 1.0 - value;

    t_matrix.setRangeMappedValuePercent(channel(), value, range());
}

void WheelRotationCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

    m_impl->wheelName = channel()->name();
    FixtureCapability::constantPercent(t_json.value("speed").toString(), &m_impl->startSpeed);
    FixtureCapability::constantPercent(t_json.value("speed").toString(), &m_impl->stopSpeed);

    FixtureCapability::constantPercent(t_json.value("speedStart").toString(), &m_impl->startSpeed);
    FixtureCapability::constantPercent(t_json.value("speedStop").toString(), &m_impl->stopSpeed);

}

} // namespace photon
