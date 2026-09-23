#ifndef PHOTON_CTOCAPABILITY_H
#define PHOTON_CTOCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

// A CTO (Color Temperature Orange) channel - a single percent axis running
// from "default" (no correction) to "CTO" (full warm shift), the same shape
// as a dimmer/intensity channel rather than part of RGB color mixing.
class PHOTONCORE_EXPORT CTOCapability : public FixtureCapability
{
public:
    CTOCapability(DMXRange range = DMXRange{});
    ~CTOCapability();

    void setPercent(double value, DMXMatrix &t_matrix, double blend = 1.0, DMXTimeMachine *timeMachine = nullptr);
    double getPercent(const DMXMatrix &t_matrix) const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CTOCAPABILITY_H
