#ifndef PHOTON_WHEELROTATIONCAPABILITY_H
#define PHOTON_WHEELROTATIONCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT WheelRotationCapability : public FixtureCapability
{
public:
    WheelRotationCapability();
    ~WheelRotationCapability();

    QString wheelName() const;
    double speedStart() const;
    double speedEnd() const;
    double maxSpeed() const;

    void setSpeed(double value, DMXMatrix &t_matrix, double t_blend);
    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_WHEELROTATIONCAPABILITY_H
