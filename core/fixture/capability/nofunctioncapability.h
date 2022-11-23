#ifndef PHOTON_NOFUNCTIONCAPABILITY_H
#define PHOTON_NOFUNCTIONCAPABILITY_H
#include "fixturecapability.h"

namespace photon {

class NoFunctionCapability : public FixtureCapability
{
public:
    NoFunctionCapability(DMXRange range = DMXRange{});
};

} // namespace photon

#endif // PHOTON_NOFUNCTIONCAPABILITY_H
