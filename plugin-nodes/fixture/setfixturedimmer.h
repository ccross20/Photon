#ifndef PHOTON_SETFIXTUREDIMMER_H
#define PHOTON_SETFIXTUREDIMMER_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class SetFixtureDimmer : public keira::Node
{
public:
    SetFixtureDimmer();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_intensityParam;
    keira::DecimalParameter *m_blendParam;
    keira::IntegerParameter *m_capabilityParam;
};

} // namespace photon



#endif // PHOTON_SETFIXTUREDIMMER_H
