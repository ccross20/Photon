#ifndef PHOTON_UVSTATE_H
#define PHOTON_UVSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT UVState : public StateCapability
{
public:
    UVState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_UVSTATE_H
