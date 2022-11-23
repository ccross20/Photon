#include <QVariant>
#include "arithmeticnode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray ArithmeticNode::InputA = "inputA";
const QByteArray ArithmeticNode::InputB = "inputB";
const QByteArray ArithmeticNode::Output = "output";

class ArithmeticNode::Impl
{
public:
    keira::DecimalParameter *inputAParam;
    keira::DecimalParameter *inputBParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation ArithmeticNode::info()
{
    keira::NodeInformation toReturn([](){return new ArithmeticNode;});
    toReturn.name = "Arithmetic";
    toReturn.nodeId = "photon.math.arithmetic";

    return toReturn;
}

ArithmeticNode::ArithmeticNode() : keira::Node("photon.math.arithmetic"),m_impl(new Impl)
{
    setName("Arithmetic");
}

ArithmeticNode::~ArithmeticNode()
{
    delete m_impl;
}

void ArithmeticNode::createParameters()
{
    m_impl->inputAParam = new keira::DecimalParameter(InputA,"Input A", 0.0);
    addParameter(m_impl->inputAParam);
    m_impl->inputBParam = new keira::DecimalParameter(InputB,"Input B", 0.0);
    addParameter(m_impl->inputBParam);

    m_impl->outputParam = new keira::DecimalParameter(Output,"Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void ArithmeticNode::evaluate(keira::EvaluationContext *t_context) const
{
    m_impl->outputParam->setValue(m_impl->inputAParam->value().toDouble() + m_impl->inputBParam->value().toDouble());

}

} // namespace photon
