#ifndef PHOTON_SETFIXTUREFOCUS_H
#define PHOTON_SETFIXTUREFOCUS_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class SetFixtureFocus : public keira::Node
{
public:
    SetFixtureFocus();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_angleParam;
    keira::DecimalParameter *m_blendParam;
    keira::IntegerParameter *m_capabilityParam;
};

} // namespace photon

#endif // PHOTON_SETFIXTUREFOCUS_H
