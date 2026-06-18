#ifndef PRISMROTATIONCAPABILITY_H
#define PRISMROTATIONCAPABILITY_H

#include "fixturecapability.h"

namespace photon {



class PHOTONCORE_EXPORT PrismRotationCapability : public FixtureCapability
{
public:
    PrismRotationCapability();
    ~PrismRotationCapability();

    bool supportsAngle() const;
    double angleStart() const;
    double angleEnd() const;
    double speedStart() const;
    double speedEnd() const;
    double maxSpeed() const;
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

#endif // PRISMROTATIONCAPABILITY_H
