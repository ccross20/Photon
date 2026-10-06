#ifndef PHOTON_GOBOSTATE_H
#define PHOTON_GOBOSTATE_H
#include "statecapability.h"


namespace photon {

// Selects a slot on a named wheel. Channels:
//   0 Wheel       - the wheel's name, matched case-insensitively against each
//                   fixture's wheels; empty means the fixture's first gobo
//                   wheel, so the default works on any fixture with one.
//   1 Slot        - 1-based slot number, as in the fixture definition.
//   2 Shake       - 0..1; above 0, shakes the slot instead of selecting it.
//   3 Rotate Mode - Any / Index / Continuous; restricts which of a slot's
//                   capabilities match when a fixture offers it both ways.
class PHOTONCORE_EXPORT GoboState : public StateCapability
{
public:
    GoboState();

    void evaluate(const StateEvaluationContext &) const override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;

private:
    QStringList rotateModeOptions;
};

} // namespace photon

#endif // PHOTON_GOBOSTATE_H
