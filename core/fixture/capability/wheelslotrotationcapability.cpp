#include "wheelslotrotationcapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixturechannel.h"

namespace photon {

class WheelSlotRotationCapability::Impl
{
public:
    QString wheel;
    double angleStart = 0;
    double angleStop = 0;
    double angleDelta = 0;
    double speedStart = 0;
    double speedStop = 0;
    bool supportsAngle = false;
};

WheelSlotRotationCapability::WheelSlotRotationCapability() : FixtureCapability(DMXRange(),Capability_WheelSlotRotation),m_impl(new Impl)
{

}

WheelSlotRotationCapability::~WheelSlotRotationCapability()
{
    delete m_impl;
}

double WheelSlotRotationCapability::speedStart() const
{
    return m_impl->speedStart;
}

double WheelSlotRotationCapability::speedEnd() const
{
    return m_impl->speedStop;
}

bool WheelSlotRotationCapability::supportsAngle() const
{
    return m_impl->supportsAngle;
}

double WheelSlotRotationCapability::angleStart() const
{
    return m_impl->angleStart;
}

double WheelSlotRotationCapability::angleEnd() const
{
    return m_impl->angleStop;
}

QString WheelSlotRotationCapability::wheel() const
{
    return m_impl->wheel;
}

void WheelSlotRotationCapability::setAngleDegrees(double value, DMXMatrix &t_matrix, double blend)
{
    double delta = (value - m_impl->angleStart) / m_impl->angleDelta;


    t_matrix.setRangeMappedValuePercent(channel(), delta, range().start,range().end);
}

void WheelSlotRotationCapability::setSpeed(double value, DMXMatrix &t_matrix, double t_blend)
{
    if(value < 0)
        value *= -1.0;

    if(m_impl->speedStop > m_impl->speedStart)
        value = 1.0 - value;

    t_matrix.setRangeMappedValuePercent(channel(), value, range());
}

void WheelSlotRotationCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);


    FixtureCapability::constantPercent(t_json.value("speedStart").toString(), &m_impl->speedStart);
    FixtureCapability::constantPercent(t_json.value("speedStop").toString(), &m_impl->speedStop);

    FixtureCapability::rotationAngle(t_json.value("angleStart").toString(), &m_impl->angleStart);
    FixtureCapability::rotationAngle(t_json.value("angleEnd").toString(), &m_impl->angleStop);

    m_impl->supportsAngle = m_impl->angleStart != m_impl->angleStop;

    m_impl->angleDelta = m_impl->angleStop - m_impl->angleStart;

    m_impl->wheel = t_json.value("wheel").toString().toLower();

}

} // namespace photon
