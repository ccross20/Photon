#ifndef PHOTON_GOBOSTATE_H
#define PHOTON_GOBOSTATE_H
#include "statecapability.h"


namespace photon {

class PHOTONCORE_EXPORT GoboState : public StateCapability
{
public:
    GoboState();

    void evaluate(const StateEvaluationContext &) const override;

private:
    QStringList wheelOptions;
};

} // namespace photon

#endif // PHOTON_GOBOSTATE_H
