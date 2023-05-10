#ifndef PHOTON_CAPABILITYFACTORY_H
#define PHOTON_CAPABILITYFACTORY_H
#include "photon-global.h"
#include "fixturecapability.h"

namespace photon {

class CapabilityFactory
{
public:
    CapabilityFactory();

    static FixtureCapability *createCapability(CapabilityType);
};

} // namespace photon

#endif // PHOTON_CAPABILITYFACTORY_H
