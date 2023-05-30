#include "trigonometrynode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray TrigonometryNode::InputA = "inputA";
const QByteArray TrigonometryNode::Mode = "mode";
const QByteArray TrigonometryNode::Output = "output";

class TrigonometryNode::Impl
{
public:
    keira::DecimalParameter *inputAParam;
    keira::OptionParameter *modeParam;
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

    m_impl->modeParam = new keira::OptionParameter(Mode,"Mode",{"Sine","Cosine","Tangent"}, 0);
    addParameter(m_impl->modeParam);

    m_impl->outputParam = new keira::DecimalParameter(Output,"Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void TrigonometryNode::evaluate(keira::EvaluationContext *t_context) const
{
    switch(m_impl->modeParam->value().toInt())
    {
        case 0:
            m_impl->outputParam->setValue(std::sin(m_impl->inputAParam->value().toDouble()));
        break;
        case 1:
            m_impl->outputParam->setValue(std::cos(m_impl->inputAParam->value().toDouble()));
        break;
        case 2:
            m_impl->outputParam->setValue(std::tan(m_impl->inputAParam->value().toDouble()));
        break;
    }





}

} // namespace photon
