#ifndef PHOTON_FOGCAPABILITY_H
#define PHOTON_FOGCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

// A fog/haze machine's output channel, used as a switch: on writes the top
// of the capability's DMX range, off writes 0. A definition can give the
// whole channel as fog (on = 255) or, as most machines need, only the upper
// part of it (e.g. 128-255 = fog, below = off).
class PHOTONCORE_EXPORT FogCapability : public FixtureCapability
{
public:
    FogCapability(DMXRange range = DMXRange{});

    void setOn(bool on, DMXMatrix &t_matrix) const;
    bool isOn(const DMXMatrix &t_matrix) const;
};

} // namespace photon

#endif // PHOTON_FOGCAPABILITY_H
