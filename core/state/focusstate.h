#ifndef PHOTON_FOCUSSTATE_H
#define PHOTON_FOCUSSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT FocusState : public StateCapability
{
public:
    FocusState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon


#endif // PHOTON_FOCUSSTATE_H
