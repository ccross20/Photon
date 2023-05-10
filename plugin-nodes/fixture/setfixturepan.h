#ifndef PHOTON_SETFIXTUREPAN_H
#define PHOTON_SETFIXTUREPAN_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class SetFixturePan : public keira::Node
{
public:
    SetFixturePan();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_angleParam;
    keira::DecimalParameter *m_blendParam;
    keira::IntegerParameter *m_modeParam;
    keira::IntegerParameter *m_capabilityParam;
};

} // namespace photon

#endif // PHOTON_SETFIXTUREPAN_H
