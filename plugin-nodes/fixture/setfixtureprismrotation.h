#ifndef SETFIXTUREPRISMROTATION_H
#define SETFIXTUREPRISMROTATION_H
#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/booleanparameter.h"

namespace photon {


class SetFixturePrismRotation : public keira::Node
{
public:
    SetFixturePrismRotation();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::OptionParameter *m_prismParam;
    keira::OptionParameter *m_modeParam;
    keira::DecimalParameter *m_angleParam;
    keira::BooleanParameter *m_usePrismParam;
};

} // namespace photon

#endif // SETFIXTUREPRISMROTATION_H
