#ifndef PHOTON_TILTANGLESTATE_H
#define PHOTON_TILTANGLESTATE_H
#include "statecapability.h"

namespace photon {

class PHOTONCORE_EXPORT TiltAngleState : public StateCapability
{
public:
    TiltAngleState();

    CapabilityType fixtureCapabilityType() const override;
    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_TILTANGLESTATE_H
