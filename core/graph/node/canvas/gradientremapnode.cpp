#include "gradientremapnode.h"
#include "graph/parameter/gradientparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/booleanparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray GradientRemapNode::Input = "input";
const QByteArray GradientRemapNode::Offset = "offset";
const QByteArray GradientRemapNode::Scale = "scale";
const QByteArray GradientRemapNode::Repeat = "repeat";
const QByteArray GradientRemapNode::Output = "output";

keira::NodeInformation GradientRemapNode::info()
{
    keira::NodeInformation toReturn([](){return new GradientRemapNode;});
    toReturn.name = "Remap Gradient";
    toReturn.nodeId = "photon.canvas.gradient-remap";
    toReturn.categories = {"Gradient"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

GradientRemapNode::GradientRemapNode() : keira::Node("photon.canvas.gradient-remap")
{
    setName("Remap Gradient");
}

void GradientRemapNode::createParameters()
{
    m_input = new GradientParameter(Input, "Gradient", Gradient{});
    addParameter(m_input);

    m_offset = new keira::DecimalParameter(Offset, "Offset", 0.0);
    addParameter(m_offset);

    m_scale = new keira::DecimalParameter(Scale, "Scale", 1.0);
    addParameter(m_scale);

    m_repeat = new keira::BooleanParameter(Repeat, "Repeat", true);
    addParameter(m_repeat);

    m_output = new GradientParameter(Output, "Gradient", Gradient{}, keira::AllowMultipleOutput);
    addParameter(m_output);
}

void GradientRemapNode::evaluate(keira::EvaluationContext *) const
{
    const Gradient in = m_input->value().value<Gradient>();
    const double offset = m_offset->value().toDouble();
    const double scale = m_scale->value().toDouble();
    const bool repeat = m_repeat->value().toBool();

    m_output->setValue(QVariant::fromValue(in.remapped(offset, scale, repeat)));
}

} // namespace photon
