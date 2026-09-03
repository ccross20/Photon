#include <cmath>
#include "absolutenode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray AbsoluteNode::Input = "input";
const QByteArray AbsoluteNode::Output = "output";

class AbsoluteNode::Impl
{
public:
    keira::DecimalParameter *inputParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation AbsoluteNode::info()
{
    keira::NodeInformation toReturn([](){return new AbsoluteNode;});
    toReturn.name = "Absolute";
    toReturn.nodeId = "photon.math.absolute";
    toReturn.categories = {"Math"};

    return toReturn;
}

AbsoluteNode::AbsoluteNode() : keira::Node("photon.math.absolute"), m_impl(new Impl)
{
    setName("Absolute");
}

AbsoluteNode::~AbsoluteNode()
{
    delete m_impl;
}

void AbsoluteNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 0.0);
    addParameter(m_impl->inputParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void AbsoluteNode::evaluate(keira::EvaluationContext *) const
{
    m_impl->outputParam->setValue(std::abs(m_impl->inputParam->value().toDouble()));
}

} // namespace photon
