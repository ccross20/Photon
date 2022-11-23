#ifndef PHOTON_TILTCAPABILITY_H
#define PHOTON_TILTCAPABILITY_H


#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT TiltCapability : public FixtureCapability
{
public:

    TiltCapability(DMXRange range = DMXRange{});
    ~TiltCapability();

    void setAngleDegrees(double value, DMXMatrix &t_matrix);
    void setAnglePercent(double value, DMXMatrix &t_matrix);
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

#endif // PHOTON_TILTCAPABILITY_H
