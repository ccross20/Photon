#include "pancapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class PanCapability::Impl
{
public:
    double angle;
    double startAngle;
    double endAngle;
};

PanCapability::PanCapability(DMXRange range) : FixtureCapability(range, Capability_Pan),m_impl(new Impl)
{

}

PanCapability::~PanCapability()
{
    delete m_impl;
}

void PanCapability::setAngleDegrees(double value, DMXMatrix &t_matrix)
{

}

void PanCapability::setAnglePercent(double value, DMXMatrix &t_matrix)
{
    t_matrix.setValuePercent(fixture()->universe()-1,channel()->universalChannelNumber(), value);
}

double PanCapability::getAnglePercent(const DMXMatrix &t_matrix)
{
    return t_matrix.valuePercent(fixture()->universe()-1,channel()->universalChannelNumber());
}

double PanCapability::angle() const
{
    return m_impl->angle;
}

double PanCapability::angleStart() const
{
    return m_impl->startAngle;
}

double PanCapability::angleEnd() const
{
    return m_impl->endAngle;
}

void PanCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
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
