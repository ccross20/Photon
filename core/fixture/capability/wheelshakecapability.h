#ifndef PHOTON_WHEELSHAKECAPABILITY_H
#define PHOTON_WHEELSHAKECAPABILITY_H

#include "wheelslotcapability.h"

namespace photon {

class PHOTONCORE_EXPORT WheelShakeCapability : public WheelSlotCapability
{
public:
    WheelShakeCapability();
    ~WheelShakeCapability();

    double shakeSpeedStart() const;
    double shakeSpeedEnd() const;

    void setSpeed(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_WHEELSHAKECAPABILITY_H
