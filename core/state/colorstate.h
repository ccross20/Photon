#ifndef PHOTON_COLORSTATE_H
#define PHOTON_COLORSTATE_H
#include "statecapability.h"

namespace photon {

class PHOTONCORE_EXPORT ColorState : public StateCapability
{
public:
    ColorState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_COLORSTATE_H
