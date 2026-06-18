#ifndef TIMER_H
#define TIMER_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/buttonparameter.h"

namespace photon {

class Timer : public keira::Node
{
public:
    Timer();    

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::IntegerParameter *m_countParam;
    keira::IntegerParameter *m_loopParam;
    keira::DecimalParameter *m_delayParam;
    keira::ButtonParameter *m_resetParam;
};

} // namespace photon

#endif // TIMER_H
