#ifndef PHOTON_SETFIXTUREZOOM_H
#define PHOTON_SETFIXTUREZOOM_H
#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class SetFixtureZoom : public keira::Node
{
public:
    SetFixtureZoom();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_angleParam;
    keira::DecimalParameter *m_blendParam;
    keira::IntegerParameter *m_capabilityParam;
};

} // namespace photon

#endif // PHOTON_SETFIXTUREZOOM_H
