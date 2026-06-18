#ifndef STOPWATCHNODE_H
#define STOPWATCHNODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class StopwatchNode : public keira::Node
{
public:
    StopwatchNode();
    ~StopwatchNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;

    keira::OptionParameter *m_modeParam;
    keira::DecimalParameter *m_triggerInputParam;
    keira::DecimalParameter *m_loopTimeInputParam;
    keira::IntegerParameter *m_countParam;
    keira::DecimalParameter *m_timeOutputParam;
};

} // namespace photon

#endif // STOPWATCHNODE_H
