#ifndef PHOTON_CTOSTATE_H
#define PHOTON_CTOSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT CTOState : public StateCapability
{
public:
    CTOState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_CTOSTATE_H
