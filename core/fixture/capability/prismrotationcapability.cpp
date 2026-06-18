#include "prismrotationcapability.h"
#include "data/dmxmatrix.h"


namespace photon {

class PrismRotationCapability::Impl
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

PrismRotationCapability::PrismRotationCapability() : FixtureCapability(DMXRange(),Capability_PrismRotation),m_impl(new Impl)
{

}

PrismRotationCapability::~PrismRotationCapability()
{
    delete m_impl;
}

double PrismRotationCapability::speedStart() const
{
    return m_impl->speedStart;
}

double PrismRotationCapability::speedEnd() const
{
    return m_impl->speedStop;
}

double PrismRotationCapability::maxSpeed() const
{
    if(m_impl->speedStop >= 0)
        return fmax(m_impl->speedStart, m_impl->speedStop);
    else
        return fmin(m_impl->speedStart, m_impl->speedStop);
}

bool PrismRotationCapability::supportsAngle() const
{
    return m_impl->supportsAngle;
}

double PrismRotationCapability::angleStart() const
{
    return m_impl->angleStart;
}

double PrismRotationCapability::angleEnd() const
{
    return m_impl->angleStop;
}

void PrismRotationCapability::selectCapability(DMXMatrix &t_matrix)
{
    t_matrix.setValue(channel()->universe() - 1, channel()->universalChannelNumber(),range().middle(),1.0);
}

bool PrismRotationCapability::isStop() const
{
    return m_impl->speedStop == 0.0 && m_impl->speedStart == 0.0;
}

void PrismRotationCapability::setAngleDegrees(double value, DMXMatrix &t_matrix, double blend)
{
    double delta = (value - m_impl->angleStart) / m_impl->angleDelta;


    t_matrix.setRangeMappedValuePercent(channel(), delta, range().start,range().end);
}

void PrismRotationCapability::setSpeed(double value, DMXMatrix &t_matrix, double t_blend)
{

    if(value > 0)
        value = 1.0 - value;

    if(value < 0)
        value *= -1.0;


    t_matrix.setRangeMappedValuePercent(channel(), value, range());
}

void PrismRotationCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

    if(t_json.contains("speedStart"))
        FixtureCapability::constantPercent(t_json.value("speedStart").toString(), &m_impl->speedStart);
    if(t_json.contains("speedStop"))
        FixtureCapability::constantPercent(t_json.value("speedStop").toString(), &m_impl->speedStop);
    if(t_json.contains("speedEnd"))
        FixtureCapability::constantPercent(t_json.value("speedEnd").toString(), &m_impl->speedStop);
    if(t_json.contains("speed"))
    {
        FixtureCapability::constantPercent(t_json.value("speed").toString(), &m_impl->speedStop);
        FixtureCapability::constantPercent(t_json.value("speed").toString(), &m_impl->speedStart);
    }

    FixtureCapability::rotationAngle(t_json.value("angleStart").toString(), &m_impl->angleStart);
    FixtureCapability::rotationAngle(t_json.value("angleEnd").toString(), &m_impl->angleStop);

    m_impl->supportsAngle = m_impl->angleStart != m_impl->angleStop;

    m_impl->angleDelta = m_impl->angleStop - m_impl->angleStart;


}

} // namespace photon
