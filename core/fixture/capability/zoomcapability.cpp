#include "zoomcapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class ZoomCapability::Impl
{
public:
    double angle;
    double startAngle;
    double endAngle;
};

ZoomCapability::ZoomCapability(DMXRange range) : FixtureCapability(range, Capability_Zoom),m_impl(new Impl)
{

}

ZoomCapability::~ZoomCapability()
{
    delete m_impl;
}

void ZoomCapability::setAngleDegrees(double value, DMXMatrix &t_matrix, double t_blend)
{

}

void ZoomCapability::setAnglePercent(double value, DMXMatrix &t_matrix, double t_blend)
{
    t_matrix.setValuePercent(fixture()->universe()-1,channel()->universalChannelNumber(), value, t_blend);
}

double ZoomCapability::getAnglePercent(const DMXMatrix &t_matrix)
{
    return t_matrix.valuePercent(fixture()->universe()-1,channel()->universalChannelNumber());
}

double ZoomCapability::angle() const
{
    return m_impl->angle;
}

double ZoomCapability::angleStart() const
{
    return m_impl->startAngle;
}

double ZoomCapability::angleEnd() const
{
    return m_impl->endAngle;
}

void ZoomCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
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
