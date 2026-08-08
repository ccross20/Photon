#ifndef PHOTON_LENSROTATIONSTATE_H
#define PHOTON_LENSROTATIONSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT LensRotationState : public StateCapability
{
public:
    LensRotationState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_LENSROTATIONSTATE_H
