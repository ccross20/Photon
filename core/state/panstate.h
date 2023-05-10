#ifndef PHOTON_PANSTATE_H
#define PHOTON_PANSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT PanState : public StateCapability
{
public:
    PanState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_PANSTATE_H
