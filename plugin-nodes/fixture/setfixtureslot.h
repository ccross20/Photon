#ifndef PHOTON_SETFIXTURESLOT_H
#define PHOTON_SETFIXTURESLOT_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/stringparameter.h"

namespace photon {

class SetFixtureSlot : public keira::Node
{
public:
    SetFixtureSlot();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();


private:
    keira::IntegerParameter *m_wheelParam;
    keira::StringParameter *m_wheelNameParam;
    keira::OptionParameter *m_modeParam;
    keira::IntegerParameter *m_slotParam;
};

} // namespace photon

#endif // PHOTON_SETFIXTURESLOT_H
