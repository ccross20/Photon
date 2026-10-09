#ifndef PHOTON_FOGSTATE_H
#define PHOTON_FOGSTATE_H
#include "statecapability.h"

namespace photon {

// Turns a fog machine's output on or off (see FogCapability).
class PHOTONCORE_EXPORT FogState : public StateCapability
{
public:
    FogState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_FOGSTATE_H
