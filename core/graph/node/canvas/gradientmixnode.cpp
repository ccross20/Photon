#include "gradientmixnode.h"
#include "graph/parameter/gradientparameter.h"
#include "model/parameter/decimalparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray GradientMixNode::InputA = "inputA";
const QByteArray GradientMixNode::InputB = "inputB";
const QByteArray GradientMixNode::Factor = "factor";
const QByteArray GradientMixNode::Output = "output";

keira::NodeInformation GradientMixNode::info()
{
    keira::NodeInformation toReturn([](){return new GradientMixNode;});
    toReturn.name = "Mix Gradients";
    toReturn.nodeId = "photon.canvas.gradient-mix";
    toReturn.categories = {"Gradient"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

GradientMixNode::GradientMixNode() : keira::Node("photon.canvas.gradient-mix")
{
    setName("Mix Gradients");
}

void GradientMixNode::createParameters()
{
    m_inputA = new GradientParameter(InputA, "Gradient A", Gradient{});
    addParameter(m_inputA);

    m_inputB = new GradientParameter(InputB, "Gradient B", Gradient{});
    addParameter(m_inputB);

    m_factor = new keira::DecimalParameter(Factor, "Factor", 0.5);
    m_factor->setMinimum(0.0);
    m_factor->setMaximum(1.0);
    addParameter(m_factor);

    m_output = new GradientParameter(Output, "Gradient", Gradient{}, keira::AllowMultipleOutput);
    addParameter(m_output);
}

void GradientMixNode::evaluate(keira::EvaluationContext *) const
{
    const Gradient a = m_inputA->value().value<Gradient>();
    const Gradient b = m_inputB->value().value<Gradient>();
    const double factor = m_factor->value().toDouble();

    m_output->setValue(QVariant::fromValue(Gradient::mixed(a, b, factor)));
}

} // namespace photon
