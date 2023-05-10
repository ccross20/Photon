#ifndef PHOTON_DIMMERCAPABILITY_H
#define PHOTON_DIMMERCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT DimmerCapability : public FixtureCapability
{
public:

    DimmerCapability(DMXRange range = DMXRange{});
    ~DimmerCapability();

    void setPercent(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setLumens(double value, DMXMatrix &t_matrix, double blend = 1.0);
    double getPercent(const DMXMatrix &t_matrix);

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DIMMERCAPABILITY_H
