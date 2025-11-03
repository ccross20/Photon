#ifndef WHEELSLOTANGLESTATE_H
#define WHEELSLOTANGLESTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT WheelSlotAngleState : public StateCapability
{
public:
    WheelSlotAngleState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon


#endif // WHEELSLOTANGLESTATE_H
