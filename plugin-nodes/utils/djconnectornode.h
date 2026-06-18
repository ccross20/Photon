#ifndef DJCONNECTORNODE_H
#define DJCONNECTORNODE_H
#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class DJConnectorNode : public keira::Node
{
public:
    DJConnectorNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::DecimalParameter *bpmParam;
    keira::DecimalParameter *beatProgressParam;
    keira::DecimalParameter *beatProgress2Param;
    keira::DecimalParameter *beatProgress4Param;
    keira::DecimalParameter *beatIntensityParam;
    keira::DecimalParameter *beatAmountParam;
    keira::IntegerParameter *beatParam;
};

} // namespace photon

#endif // DJCONNECTORNODE_H
