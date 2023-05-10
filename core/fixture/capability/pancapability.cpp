#include "pancapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixturechannel.h"

namespace photon {

class PanCapability::Impl
{
public:
    double angle;
    double startAngle =  0.0;
    double endAngle = 360.0;
};

PanCapability::PanCapability(DMXRange range) : FixtureCapability(range, Capability_Pan),m_impl(new Impl)
{

}

PanCapability::~PanCapability()
{
    delete m_impl;
}

void PanCapability::setAngleDegrees(double value, DMXMatrix &t_matrix, double t_blend)
{
    value = std::max(std::min(value, m_impl->endAngle), m_impl->startAngle);

    double percent = (value - m_impl->startAngle) / (m_impl->endAngle - m_impl->startAngle);
    t_matrix.setValuePercent(channel(), percent, t_blend);
}

void PanCapability::setAngleDegreesCentered(double value, DMXMatrix &t_matrix, double t_blend)
{
    double offset = (m_impl->endAngle - m_impl->startAngle) / 2.0;
    setAngleDegrees(value + offset, t_matrix, t_blend);
}


void PanCapability::setAnglePercent(double value, DMXMatrix &t_matrix, double t_blend)
{
    t_matrix.setValuePercent(channel(), value, t_blend);
}

double PanCapability::getAnglePercent(const DMXMatrix &t_matrix)
{
    return t_matrix.valuePercent(channel());
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
    qDebug() << "pan";

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
        qDebug() << unit;

        if(unit == Unit_Percent)
        {

        }
        if(unit == Unit_Degrees)
        {
            m_impl->endAngle = value;
            qDebug() << "End angle" << value;
        }
    }
}


} // namespace photon
