#ifndef PHOTON_SETFIXTURECOLOR_H
#define PHOTON_SETFIXTURECOLOR_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

class SetFixtureColor : public keira::Node
{
public:
    SetFixtureColor();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    ColorParameter *m_colorParam;
    keira::DecimalParameter *m_blendParam;
    keira::IntegerParameter *m_capabilityParam;
};

} // namespace photon

#endif // PHOTON_SETFIXTURECOLOR_H
