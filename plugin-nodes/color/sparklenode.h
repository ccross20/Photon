#ifndef SPARKLENODE_H
#define SPARKLENODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/booleanparameter.h"
#include "graph/parameter/colorparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

class SparkleNode : public keira::Node
{
public:
    SparkleNode();
    ~SparkleNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;

    ColorParameter *m_colorParam;
    keira::OptionParameter *m_easeInTypeParam;
    keira::DecimalParameter *m_easeInDurationParam;
    keira::OptionParameter *m_easeOutTypeParam;
    keira::DecimalParameter *m_easeOutDurationParam;
    keira::IntegerParameter *m_seedParam;
    keira::DecimalParameter *m_spreadParam;
    keira::DecimalParameter *m_gapParam;
    keira::DecimalParameter *m_middleDurationParam;

};

} // namespace photon

#endif // SPARKLENODE_H
