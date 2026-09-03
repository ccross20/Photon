#include <QtGlobal>
#include "anglecapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixturechannel.h"

namespace photon {

class AngleCapability::Impl
{
public:
    double angle = 0.0;
    double startAngle = 0.0;
    double endAngle = 0.0;
    // Fractional beam openness at the ends of the DMX range (see
    // directedPercent). Default 0 -> 1: low DMX = tight, high DMX = wide.
    double startPercent = 0.0;
    double endPercent = 1.0;
};

AngleCapability::AngleCapability(CapabilityType t_capability, DMXRange t_range) : FixtureCapability(t_range, t_capability),m_impl(new Impl)
{

}

AngleCapability::~AngleCapability()
{
    delete m_impl;
}

void AngleCapability::setAngleDegrees(double value, DMXMatrix &t_matrix, double t_blend)
{
    value = std::max(std::min(value, m_impl->endAngle), m_impl->startAngle);

    double percent = (value - m_impl->startAngle) / (m_impl->endAngle - m_impl->startAngle);
    t_matrix.setValuePercent(channel(), percent, t_blend);
}

void AngleCapability::setAngleDegreesCentered(double value, DMXMatrix &t_matrix, double t_blend)
{
    double offset = (m_impl->endAngle - m_impl->startAngle) / 2.0;
    setAngleDegrees(value + offset, t_matrix, t_blend);
}


void AngleCapability::setAnglePercent(double value, DMXMatrix &t_matrix, double t_blend)
{
    t_matrix.setValuePercent(channel(), value, t_blend);
}

void AngleCapability::setAnglePercentCentered(double value, DMXMatrix &t_matrix, double t_blend)
{
    t_matrix.setValuePercent(channel(), .5 + value/2.0, t_blend);
}

double AngleCapability::getAnglePercent(const DMXMatrix &t_matrix) const
{
    return t_matrix.valuePercent(channel());
}

double AngleCapability::directedPercent(const DMXMatrix &t_matrix) const
{
    const double raw = getAnglePercent(t_matrix);
    return m_impl->startPercent + raw * (m_impl->endPercent - m_impl->startPercent);
}

double AngleCapability::angle() const
{
    return m_impl->angle;
}

double AngleCapability::angleStart() const
{
    return m_impl->startAngle;
}

double AngleCapability::angleEnd() const
{
    return m_impl->endAngle;
}

void AngleCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
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
            m_impl->startPercent = qBound(0.0, value, 1.0);
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
            m_impl->endPercent = qBound(0.0, value, 1.0);
        }
        if(unit == Unit_Degrees)
        {
            m_impl->endAngle = value;
        }
    }
}


} // namespace photon
