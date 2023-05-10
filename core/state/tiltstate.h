#ifndef PHOTON_TILTSTATE_H
#define PHOTON_TILTSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT TiltState : public StateCapability
{
public:
    TiltState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_TILTSTATE_H
