#include "invertnode.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

const QByteArray InvertNode::Input = "input";
const QByteArray InvertNode::Output = "output";

class InvertNode::Impl
{
public:
    keira::BooleanParameter *inputParam;
    keira::BooleanParameter *outputParam;
};

keira::NodeInformation InvertNode::info()
{
    keira::NodeInformation toReturn([](){return new InvertNode;});
    toReturn.name = "Invert";
    toReturn.nodeId = "photon.math.invert";
    toReturn.categories = {"Math"};

    return toReturn;
}

InvertNode::InvertNode() : keira::Node("photon.math.invert"), m_impl(new Impl)
{
    setName("Invert");
}

InvertNode::~InvertNode()
{
    delete m_impl;
}

void InvertNode::createParameters()
{
    m_impl->inputParam = new keira::BooleanParameter(Input, "Input", false);
    addParameter(m_impl->inputParam);

    m_impl->outputParam = new keira::BooleanParameter(Output, "Output", true, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void InvertNode::evaluate(keira::EvaluationContext *) const
{
    m_impl->outputParam->setValue(!m_impl->inputParam->value().toBool());
}

} // namespace photon
