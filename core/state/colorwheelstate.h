#ifndef PHOTON_COLORWHEELSTATE_H
#define PHOTON_COLORWHEELSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT ColorWheelState : public StateCapability
{
public:
    ColorWheelState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_COLORWHEELSTATE_H
