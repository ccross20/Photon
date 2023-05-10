#ifndef PHOTON_ZOOMSTATE_H
#define PHOTON_ZOOMSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT ZoomState : public StateCapability
{
public:
    ZoomState();

    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_ZOOMSTATE_H
