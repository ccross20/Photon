#ifndef PHOTON_SETFIXTURESTROBE_H
#define PHOTON_SETFIXTURESTROBE_H
#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"


namespace photon {

class SetFixtureStrobe : public keira::Node
{
public:
    SetFixtureStrobe();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_speedParam;
    keira::IntegerParameter *m_modeParam;
};

} // namespace photon

#endif // PHOTON_SETFIXTURESTROBE_H
