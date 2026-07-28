#include "gradientsamplenode.h"
#include "graph/parameter/gradientparameter.h"
#include "graph/parameter/colorparameter.h"
#include "model/parameter/decimalparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray GradientSampleNode::Input = "input";
const QByteArray GradientSampleNode::Position = "position";
const QByteArray GradientSampleNode::Output = "output";

keira::NodeInformation GradientSampleNode::info()
{
    keira::NodeInformation toReturn([](){return new GradientSampleNode;});
    toReturn.name = "Sample Gradient";
    toReturn.nodeId = "photon.canvas.gradient-sample";
    toReturn.categories = {"Gradient"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

GradientSampleNode::GradientSampleNode() : keira::Node("photon.canvas.gradient-sample")
{
    setName("Sample Gradient");
}

void GradientSampleNode::createParameters()
{
    m_input = new GradientParameter(Input, "Gradient", Gradient{});
    addParameter(m_input);

    m_position = new keira::DecimalParameter(Position, "Position", 0.5);
    m_position->setMinimum(0.0);
    m_position->setMaximum(1.0);
    addParameter(m_position);

    m_output = new ColorParameter(Output, "Color", QColor(Qt::black), keira::AllowMultipleOutput);
    addParameter(m_output);
}

void GradientSampleNode::evaluate(keira::EvaluationContext *) const
{
    const Gradient in = m_input->value().value<Gradient>();
    m_output->setValue(in.colorAt(m_position->value().toDouble()));
}

} // namespace photon
