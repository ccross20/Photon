#ifndef PHOTON_WHEELSLOTROTATIONCAPABILITY_H
#define PHOTON_WHEELSLOTROTATIONCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT WheelSlotRotationCapability : public FixtureCapability
{
public:
    // Reused for other continuous rotation channels that share this shape
    // (index-or-speed, optional wheel-name binding) - e.g. Capability_LensRotation
    // for a bee-eye's rotating lens plate. Mirrors how AngleCapability serves
    // Pan/Tilt/Zoom/Focus via a constructor tag.
    WheelSlotRotationCapability(CapabilityType type = Capability_WheelSlotRotation);
    ~WheelSlotRotationCapability();

    bool supportsAngle() const;
    double angleStart() const;
    double angleEnd() const;
    double speedStart() const;
    double speedEnd() const;
    double maxSpeed() const;
    QString wheel() const;
    bool isStop() const;

    void selectCapability(DMXMatrix &t_matrix);
    void setSpeed(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setAngleDegrees(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_WHEELSLOTROTATIONCAPABILITY_H
