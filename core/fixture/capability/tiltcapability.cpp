#include "tiltcapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class TiltCapability::Impl
{
public:
    double angle;
    double startAngle;
    double endAngle;
};

TiltCapability::TiltCapability(DMXRange range) : FixtureCapability(range, Capability_Tilt),m_impl(new Impl)
{

}

TiltCapability::~TiltCapability()
{
    delete m_impl;
}

void TiltCapability::setAngleDegrees(double value, DMXMatrix &t_matrix)
{

}

void TiltCapability::setAnglePercent(double value, DMXMatrix &t_matrix)
{
    t_matrix.setValuePercent(fixture()->universe()-1,channel()->universalChannelNumber(), value);
}

double TiltCapability::getAnglePercent(const DMXMatrix &t_matrix)
{
    return t_matrix.valuePercent(fixture()->universe()-1,channel()->universalChannelNumber());
}

double TiltCapability::angle() const
{
    return m_impl->angle;
}

double TiltCapability::angleStart() const
{
    return m_impl->startAngle;
}

double TiltCapability::angleEnd() const
{
    return m_impl->endAngle;
}

void TiltCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

    if(t_json.contains("angle"))
    {
        double value;
        auto unit = FixtureCapability::rotationAngle(t_json.value("angle").toString(), &value);

        if(unit == Unit_Percent)
        {

        }
        if(unit == Unit_Degrees)
        {
            m_impl->angle = value;
        }
    }

    if(t_json.contains("angleStart"))
    {
        double value;
        auto unit = FixtureCapability::rotationAngle(t_json.value("angleStart").toString(), &value);

        if(unit == Unit_Percent)
        {

        }
        if(unit == Unit_Degrees)
        {
            m_impl->startAngle = value;
        }
    }

    if(t_json.contains("angleEnd"))
    {
        double value;
        auto unit = FixtureCapability::rotationAngle(t_json.value("angleEnd").toString(), &value);

        if(unit == Unit_Percent)
        {

        }
        if(unit == Unit_Degrees)
        {
            m_impl->endAngle = value;
        }
    }
}


} // namespace photon
