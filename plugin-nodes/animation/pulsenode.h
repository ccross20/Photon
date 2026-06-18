#ifndef PULSENODE_H
#define PULSENODE_H
#include <QEasingCurve>
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

class PulseNode : public keira::Node
{
public:
    PulseNode();
    ~PulseNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;

    keira::DecimalParameter *m_timeInputParam;
    keira::OptionParameter *m_easeInTypeParam;
    keira::DecimalParameter *m_easeInDurationParam;
    keira::OptionParameter *m_easeOutTypeParam;
    keira::DecimalParameter *m_easeOutDurationParam;
    keira::IntegerParameter *m_loopCountParam;
    keira::DecimalParameter *m_amplitudeParam;
    keira::DecimalParameter *m_durationParam;
    keira::DecimalParameter *m_gapParam;
    keira::DecimalParameter *m_outputParam;
};

} // namespace photon

#endif // PULSENODE_H
