#ifndef PHOTON_DIMMERSTATE_H
#define PHOTON_DIMMERSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT DimmerState : public StateCapability
{
public:
    DimmerState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_DIMMERSTATE_H
