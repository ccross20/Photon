#ifndef PHOTON_WHEELROTATIONSTATE_H
#define PHOTON_WHEELROTATIONSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT WheelRotationState : public StateCapability
{
public:
    WheelRotationState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_WHEELROTATIONSTATE_H
