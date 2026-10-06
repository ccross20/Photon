#ifndef PHOTON_COLORINTENSITYCAPABILITY_H
#define PHOTON_COLORINTENSITYCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT ColorIntensityCapability : public FixtureCapability
{
public:
    ColorIntensityCapability(CapabilityType capability = Capability_Color, DMXRange range = DMXRange{});

    void setPercent(double value, DMXMatrix &t_matrix, double t_blend = 1.0) const;
    double getPercent(const DMXMatrix &t_matrix) const;    

    void readFromOpenFixtureJson(const QJsonObject &) override;

};

} // namespace photon

#endif // PHOTON_COLORINTENSITYCAPABILITY_H
