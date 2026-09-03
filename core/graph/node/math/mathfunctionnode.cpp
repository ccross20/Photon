#include <cmath>
#include "mathfunctionnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray MathFunctionNode::Input = "input";
const QByteArray MathFunctionNode::ModeInput = "mode";
const QByteArray MathFunctionNode::Output = "output";

class MathFunctionNode::Impl
{
public:
    keira::DecimalParameter *inputParam;
    keira::OptionParameter *modeParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation MathFunctionNode::info()
{
    keira::NodeInformation toReturn([](){return new MathFunctionNode;});
    toReturn.name = "Math Function";
    toReturn.nodeId = "photon.math.function";
    toReturn.categories = {"Math"};

    return toReturn;
}

MathFunctionNode::MathFunctionNode() : keira::Node("photon.math.function"), m_impl(new Impl)
{
    setName("Math Function");
}

MathFunctionNode::~MathFunctionNode()
{
    delete m_impl;
}

void MathFunctionNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 0.0);
    addParameter(m_impl->inputParam);

    // Appended, never reordered - the stored value is the option index.
    m_impl->modeParam = new keira::OptionParameter(ModeInput, "Mode", {
        "Square",
        "Sign",
        "Negate",
        "Reciprocal",
        "Fractional",
        "Exponential",
    }, ModeSquare);
    addParameter(m_impl->modeParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void MathFunctionNode::evaluate(keira::EvaluationContext *) const
{
    const double in = m_impl->inputParam->value().toDouble();

    double result = in;
    switch(m_impl->modeParam->value().toInt())
    {
    case ModeSquare:      result = in * in; break;
    case ModeSign:        result = (in > 0.0) - (in < 0.0); break;   // -1 / 0 / 1
    case ModeNegate:      result = -in; break;
    // 1/0 has no answer; 0 keeps it from poisoning downstream, matching how
    // ArithmeticNode handles a zero divisor.
    case ModeReciprocal:  result = qFuzzyIsNull(in) ? 0.0 : 1.0 / in; break;
    case ModeFractional:  result = in - std::trunc(in); break;       // sign follows input
    case ModeExponential: result = std::exp(in); break;
    default:              break;
    }

    m_impl->outputParam->setValue(result);
}

} // namespace photon
