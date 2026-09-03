#include <algorithm>
#include "minmaxnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray MinMaxNode::InputA = "inputA";
const QByteArray MinMaxNode::InputB = "inputB";
const QByteArray MinMaxNode::ModeInput = "mode";
const QByteArray MinMaxNode::Output = "output";

class MinMaxNode::Impl
{
public:
    keira::DecimalParameter *inputAParam;
    keira::DecimalParameter *inputBParam;
    keira::OptionParameter *modeParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation MinMaxNode::info()
{
    keira::NodeInformation toReturn([](){return new MinMaxNode;});
    toReturn.name = "Min / Max";
    toReturn.nodeId = "photon.math.min-max";
    toReturn.categories = {"Math"};

    return toReturn;
}

MinMaxNode::MinMaxNode() : keira::Node("photon.math.min-max"), m_impl(new Impl)
{
    setName("Min / Max");
}

MinMaxNode::~MinMaxNode()
{
    delete m_impl;
}

void MinMaxNode::createParameters()
{
    m_impl->inputAParam = new keira::DecimalParameter(InputA, "Input A", 0.0);
    addParameter(m_impl->inputAParam);
    m_impl->inputBParam = new keira::DecimalParameter(InputB, "Input B", 0.0);
    addParameter(m_impl->inputBParam);

    // Appended, never reordered - the stored value is the option index.
    m_impl->modeParam = new keira::OptionParameter(ModeInput, "Mode", {"Min", "Max"}, ModeMin);
    addParameter(m_impl->modeParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void MinMaxNode::evaluate(keira::EvaluationContext *) const
{
    const double a = m_impl->inputAParam->value().toDouble();
    const double b = m_impl->inputBParam->value().toDouble();

    m_impl->outputParam->setValue(m_impl->modeParam->value().toInt() == ModeMax
                                      ? std::max(a, b)
                                      : std::min(a, b));
}

} // namespace photon
