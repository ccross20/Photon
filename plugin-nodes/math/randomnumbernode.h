#ifndef RANDOMNUMBERNODE_H
#define RANDOMNUMBERNODE_H
#include <QRandomGenerator>
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

class RandomNumberNode : public keira::Node
{
public:
    RandomNumberNode();
    ~RandomNumberNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;

    keira::OptionParameter *m_modeParam;
    keira::IntegerParameter *m_seedParam;
    keira::BooleanParameter *m_ensureDifferent;
    keira::DecimalParameter *m_minParam;
    keira::DecimalParameter *m_maxParam;
    keira::DecimalParameter *m_outputParam;

};

} // namespace photon

#endif // RANDOMNUMBERNODE_H
