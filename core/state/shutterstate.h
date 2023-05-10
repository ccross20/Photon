#ifndef PHOTON_SHUTTERSTATE_H
#define PHOTON_SHUTTERSTATE_H
#include "statecapability.h"

namespace photon {

class PHOTONCORE_EXPORT ShutterState : public StateCapability
{
public:
    ShutterState();


    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_SHUTTERSTATE_H
