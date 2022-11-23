#include "trigonometrynode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray TrigonometryNode::InputA = "inputA";
const QByteArray TrigonometryNode::Output = "output";

class TrigonometryNode::Impl
{
public:
    keira::DecimalParameter *inputAParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation TrigonometryNode::info()
{
    keira::NodeInformation toReturn([](){return new TrigonometryNode;});
    toReturn.name = "Trigonometry";
    toReturn.nodeId = "photon.math.trigonometry";

    return toReturn;
}

TrigonometryNode::TrigonometryNode() : keira::Node("photon.math.trigonometry"),m_impl(new Impl)
{
    setName("Trigonometry");
}

TrigonometryNode::~TrigonometryNode()
{
    delete m_impl;
}

void TrigonometryNode::createParameters()
{
    m_impl->inputAParam = new keira::DecimalParameter(InputA,"Input A", 0.0);
    addParameter(m_impl->inputAParam);

    m_impl->outputParam = new keira::DecimalParameter(Output,"Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void TrigonometryNode::evaluate(keira::EvaluationContext *t_context) const
{
    m_impl->outputParam->setValue((std::cos(m_impl->inputAParam->value().toDouble())+1.0)/2.0);

}

} // namespace photon
