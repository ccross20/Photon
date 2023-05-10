#ifndef PHOTON_TILTANGLECENTEREDSTATE_H
#define PHOTON_TILTANGLECENTEREDSTATE_H
#include "statecapability.h"

namespace photon {

class PHOTONCORE_EXPORT TiltAngleCenteredState : public StateCapability
{
public:
    TiltAngleCenteredState();

    CapabilityType fixtureCapabilityType() const override;
    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_TILTANGLECENTEREDSTATE_H
