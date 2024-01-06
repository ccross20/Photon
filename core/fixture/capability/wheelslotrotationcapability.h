#ifndef PHOTON_WHEELSLOTROTATIONCAPABILITY_H
#define PHOTON_WHEELSLOTROTATIONCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT WheelSlotRotationCapability : public FixtureCapability
{
public:
    WheelSlotRotationCapability();
    ~WheelSlotRotationCapability();

    bool supportsAngle() const;
    double angleStart() const;
    double angleEnd() const;
    double speedStart() const;
    double speedEnd() const;
    QString wheel() const;

    void setSpeed(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setAngleDegrees(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_WHEELSLOTROTATIONCAPABILITY_H
