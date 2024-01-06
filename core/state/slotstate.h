#ifndef PHOTON_SLOTSTATE_H
#define PHOTON_SLOTSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT SlotState : public StateCapability
{
public:
    SlotState();

    void evaluate(const StateEvaluationContext &) const override;

private:
    QStringList wheelOptions;
};

} // namespace photon

#endif // PHOTON_SLOTSTATE_H
