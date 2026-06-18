#ifndef RANDOMFALLOFFNODE_H
#define RANDOMFALLOFFNODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/booleanparameter.h"
#include "photon-global.h"

namespace photon {

class RandomFalloffNode : public keira::Node
{
public:
    RandomFalloffNode();
    ~RandomFalloffNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();
private:
    class Impl;
    Impl *m_impl;

    FixtureListParameter *m_inParam;
    FixtureListParameter *m_outParam;
    keira::IntegerParameter *m_seedParam;
    keira::BooleanParameter *m_equalDistributionParam;
    keira::DecimalParameter *m_totalOffsetParam;
};

} // namespace photon

#endif // RANDOMFALLOFFNODE_H
