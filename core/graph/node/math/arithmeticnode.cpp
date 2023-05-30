#include <QVariant>
#include "arithmeticnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray ArithmeticNode::InputA = "inputA";
const QByteArray ArithmeticNode::InputB = "inputB";
const QByteArray ArithmeticNode::Mode = "mode";
const QByteArray ArithmeticNode::Output = "output";

class ArithmeticNode::Impl
{
public:
    keira::DecimalParameter *inputAParam;
    keira::DecimalParameter *inputBParam;
    keira::OptionParameter *modeParam;
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
    m_impl->modeParam = new keira::OptionParameter(Mode,"Mode",{"Add","Subtract","Multiply","Divide"}, 0);
    addParameter(m_impl->modeParam);

    m_impl->outputParam = new keira::DecimalParameter(Output,"Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void ArithmeticNode::evaluate(keira::EvaluationContext *t_context) const
{
    switch(m_impl->modeParam->value().toInt())
    {
        case 0:
            m_impl->outputParam->setValue(m_impl->inputAParam->value().toDouble() + m_impl->inputBParam->value().toDouble());
        break;
        case 1:
            m_impl->outputParam->setValue(m_impl->inputAParam->value().toDouble() - m_impl->inputBParam->value().toDouble());
        break;
        case 2:
            m_impl->outputParam->setValue(m_impl->inputAParam->value().toDouble() * m_impl->inputBParam->value().toDouble());
        break;
        case 3:
        {
            auto b = m_impl->inputBParam->value().toDouble();
            if(qFuzzyCompare(b, 0.0))
                m_impl->outputParam->setValue(0.0);
            else
                m_impl->outputParam->setValue(m_impl->inputAParam->value().toDouble() / b);
        }
        break;
    }



}

} // namespace photon
