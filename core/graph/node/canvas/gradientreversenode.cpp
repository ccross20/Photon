#include "gradientreversenode.h"
#include "graph/parameter/gradientparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray GradientReverseNode::Input = "input";
const QByteArray GradientReverseNode::Output = "output";

keira::NodeInformation GradientReverseNode::info()
{
    keira::NodeInformation toReturn([](){return new GradientReverseNode;});
    toReturn.name = "Reverse Gradient";
    toReturn.nodeId = "photon.canvas.gradient-reverse";
    toReturn.categories = {"Gradient"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

GradientReverseNode::GradientReverseNode() : keira::Node("photon.canvas.gradient-reverse")
{
    setName("Reverse Gradient");
}

void GradientReverseNode::createParameters()
{
    m_input = new GradientParameter(Input, "Gradient", Gradient{});
    addParameter(m_input);

    m_output = new GradientParameter(Output, "Gradient", Gradient{}, keira::AllowMultipleOutput);
    addParameter(m_output);
}

void GradientReverseNode::evaluate(keira::EvaluationContext *) const
{
    const Gradient in = m_input->value().value<Gradient>();
    m_output->setValue(QVariant::fromValue(in.reversed()));
}

} // namespace photon
