#include "colorintensitycapability.h"
#include "data/dmxmatrix.h"

namespace photon {

ColorIntensityCapability::ColorIntensityCapability(CapabilityType t_capability, DMXRange t_range) : FixtureCapability(t_range, t_capability)
{

}

void ColorIntensityCapability::setPercent(double value, DMXMatrix &t_matrix, double t_blend) const
{
    t_matrix.setValuePercent(channel(), value, t_blend);
}

double ColorIntensityCapability::getPercent(const DMXMatrix &t_matrix) const
{
    return t_matrix.valuePercent(channel());
}

void ColorIntensityCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);
}

} // namespace photon
