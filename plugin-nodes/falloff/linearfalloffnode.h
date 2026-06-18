#ifndef LINEARFALLOFFNODE_H
#define LINEARFALLOFFNODE_H
#include "model/node.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/decimalparameter.h"
#include "photon-global.h"

namespace photon {

class LinearFalloffNode : public keira::Node
{
public:
    LinearFalloffNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();
private:
    FixtureListParameter *m_inParam;
    FixtureListParameter *m_outParam;
    keira::BooleanParameter *m_reverseParam;
    keira::DecimalParameter *m_totalOffsetParam;
};

} // namespace photon

#endif // LINEARFALLOFFNODE_H
