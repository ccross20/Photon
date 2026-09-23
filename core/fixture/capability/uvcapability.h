#ifndef PHOTON_UVCAPABILITY_H
#define PHOTON_UVCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

// A UV/blacklight channel - a single on/off-or-intensity percent axis, the
// same shape as a dimmer/CTO channel rather than part of RGB color mixing
// (UV doesn't visibly blend with the other LEDs the way White/Amber/Lime do).
class PHOTONCORE_EXPORT UVCapability : public FixtureCapability
{
public:
    UVCapability(DMXRange range = DMXRange{});
    ~UVCapability();

    void setPercent(double value, DMXMatrix &t_matrix, double blend = 1.0, DMXTimeMachine *timeMachine = nullptr);
    double getPercent(const DMXMatrix &t_matrix) const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_UVCAPABILITY_H
