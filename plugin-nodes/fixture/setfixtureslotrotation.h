#ifndef SETFIXTURESLOTROTATION_H
#define SETFIXTURESLOTROTATION_H
#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/stringparameter.h"

namespace photon {

class SetFixtureSlotRotation : public keira::Node
{
public:
    SetFixtureSlotRotation();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::OptionParameter *m_modeParam;
    keira::OptionParameter *m_wheelParam;
    keira::DecimalParameter *m_angleParam;
};

} // namespace photon

#endif // SETFIXTURESLOTROTATION_H
