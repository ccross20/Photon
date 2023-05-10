#ifndef PHOTON_ZOOMCAPABILITY_H
#define PHOTON_ZOOMCAPABILITY_H
#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT ZoomCapability : public FixtureCapability
{
public:

    ZoomCapability(DMXRange range = DMXRange{});
    ~ZoomCapability();

    void setAngleDegrees(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setAnglePercent(double value, DMXMatrix &t_matrix, double blend = 1.0);
    double getAnglePercent(const DMXMatrix &t_matrix);

    double angle() const;
    double angleStart() const;
    double angleEnd() const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ZOOMCAPABILITY_H
