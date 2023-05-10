#include "capabilityfactory.h"
#include "dimmercapability.h"
#include "pancapability.h"
#include "shutterstrobecapability.h"
#include "tiltcapability.h"
#include "nofunctioncapability.h"

namespace photon {

CapabilityFactory::CapabilityFactory()
{

}

FixtureCapability *CapabilityFactory::createCapability(CapabilityType t_type)
{
    switch (t_type) {
        case Capability_Dimmer:
            return new DimmerCapability;
        case Capability_Pan:
            return  new PanCapability;
        case Capability_Tilt:
            return new TiltCapability;
        case Capability_Strobe:
            return new ShutterStrobeCapability;
        default:
            return new NoFunctionCapability;
    }
}

} // namespace photon
