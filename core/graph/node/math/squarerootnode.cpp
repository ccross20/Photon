#include <cmath>
#include "squarerootnode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray SquareRootNode::Input = "input";
const QByteArray SquareRootNode::Output = "output";

class SquareRootNode::Impl
{
public:
    keira::DecimalParameter *inputParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation SquareRootNode::info()
{
    keira::NodeInformation toReturn([](){return new SquareRootNode;});
    toReturn.name = "Square Root";
    toReturn.nodeId = "photon.math.square-root";
    toReturn.categories = {"Math"};

    return toReturn;
}

SquareRootNode::SquareRootNode() : keira::Node("photon.math.square-root"), m_impl(new Impl)
{
    setName("Square Root");
}

SquareRootNode::~SquareRootNode()
{
    delete m_impl;
}

void SquareRootNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 0.0);
    addParameter(m_impl->inputParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void SquareRootNode::evaluate(keira::EvaluationContext *) const
{
    const double in = m_impl->inputParam->value().toDouble();
    m_impl->outputParam->setValue(in > 0.0 ? std::sqrt(in) : 0.0);
}

} // namespace photon
