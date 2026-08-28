#ifndef PHOTON_PRISMSTATE_H
#define PHOTON_PRISMSTATE_H

#include "statecapability.h"

namespace photon {

// Engages a prism and drives its rotation. Covers both Capability_Prism (the
// on/off gate some fixtures put the prism behind) and Capability_PrismRotation
// in one state, the same pairing the Set Fixture Prism Rotation node uses.
class PHOTONCORE_EXPORT PrismState : public StateCapability
{
public:
    PrismState();

    void evaluate(const StateEvaluationContext &) const override;

private:
    QStringList modeOptions;
    QStringList prismOptions;
};

} // namespace photon

#endif // PHOTON_PRISMSTATE_H
