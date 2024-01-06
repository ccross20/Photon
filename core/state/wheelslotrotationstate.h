#ifndef PHOTON_WHEELSLOTROTATIONSTATE_H
#define PHOTON_WHEELSLOTROTATIONSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT WheelSlotRotationState : public StateCapability
{
public:
    WheelSlotRotationState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_WHEELSLOTROTATIONSTATE_H
