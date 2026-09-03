#ifndef PHOTON_ANGLECAPABILITY_H
#define PHOTON_ANGLECAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT AngleCapability : public FixtureCapability
{
public:

    AngleCapability(CapabilityType capability = Capability_Unknown, DMXRange range = DMXRange{});
    ~AngleCapability();

    void setAngleDegrees(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setAngleDegreesCentered(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setAnglePercent(double value, DMXMatrix &t_matrix, double blend = 1.0);
    void setAnglePercentCentered(double value, DMXMatrix &t_matrix, double blend = 1.0);
    double getAnglePercent(const DMXMatrix &t_matrix) const;

    // Like getAnglePercent(), but remapped through the capability's declared
    // direction: OFL angleStart/angleEnd (as fractions/keywords - "narrow",
    // "wide", "50%") give the beam "openness" at the low and high ends of the
    // DMX range. Defaults to a straight 0 -> 1 pass-through, so fixtures that
    // don't declare it (or declare the usual narrow -> wide) are unchanged;
    // a fixture wired the other way (e.g. Betopper Bee Eye: angleStart "wide",
    // angleEnd "narrow") gets its zoom flipped here. Consumers scale the result
    // across the physical lens range.
    double directedPercent(const DMXMatrix &t_matrix) const;

    double angle() const;
    double angleStart() const;
    double angleEnd() const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ANGLECAPABILITY_H
