#include <cmath>
#include "logarithmnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray LogarithmNode::Input = "input";
const QByteArray LogarithmNode::ModeInput = "mode";
const QByteArray LogarithmNode::Output = "output";

class LogarithmNode::Impl
{
public:
    keira::DecimalParameter *inputParam;
    keira::OptionParameter *modeParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation LogarithmNode::info()
{
    keira::NodeInformation toReturn([](){return new LogarithmNode;});
    toReturn.name = "Logarithm";
    toReturn.nodeId = "photon.math.logarithm";
    toReturn.categories = {"Math"};

    return toReturn;
}

LogarithmNode::LogarithmNode() : keira::Node("photon.math.logarithm"), m_impl(new Impl)
{
    setName("Logarithm");
}

LogarithmNode::~LogarithmNode()
{
    delete m_impl;
}

void LogarithmNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 1.0);
    addParameter(m_impl->inputParam);

    // Appended, never reordered - the stored value is the option index.
    m_impl->modeParam = new keira::OptionParameter(ModeInput, "Mode",
                                                   {"Natural (ln)", "Base 10", "Base 2"}, ModeNatural);
    addParameter(m_impl->modeParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void LogarithmNode::evaluate(keira::EvaluationContext *) const
{
    const double in = m_impl->inputParam->value().toDouble();
    if(in <= 0.0)
    {
        m_impl->outputParam->setValue(0.0);
        return;
    }

    double result = std::log(in);
    switch(m_impl->modeParam->value().toInt())
    {
    case ModeBase10: result = std::log10(in); break;
    case ModeBase2:  result = std::log2(in);  break;
    case ModeNatural:
    default:         result = std::log(in);   break;
    }

    m_impl->outputParam->setValue(result);
}

} // namespace photon
