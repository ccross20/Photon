#include <cmath>
#include "roundnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray RoundNode::Input = "input";
const QByteArray RoundNode::ModeInput = "mode";
const QByteArray RoundNode::Output = "output";

class RoundNode::Impl
{
public:
    keira::DecimalParameter *inputParam;
    keira::OptionParameter *modeParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation RoundNode::info()
{
    keira::NodeInformation toReturn([](){return new RoundNode;});
    toReturn.name = "Round";
    toReturn.nodeId = "photon.math.round";
    toReturn.categories = {"Math"};

    return toReturn;
}

RoundNode::RoundNode() : keira::Node("photon.math.round"), m_impl(new Impl)
{
    setName("Round");
}

RoundNode::~RoundNode()
{
    delete m_impl;
}

void RoundNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 0.0);
    addParameter(m_impl->inputParam);

    // Appended, never reordered - the stored value is the option index.
    m_impl->modeParam = new keira::OptionParameter(ModeInput, "Mode",
                                                   {"Round", "Floor", "Ceiling"}, ModeRound);
    addParameter(m_impl->modeParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void RoundNode::evaluate(keira::EvaluationContext *) const
{
    const double in = m_impl->inputParam->value().toDouble();

    double result = in;
    switch(m_impl->modeParam->value().toInt())
    {
    case ModeFloor:   result = std::floor(in); break;
    case ModeCeiling: result = std::ceil(in);  break;
    case ModeRound:
    default:          result = std::round(in); break;
    }

    m_impl->outputParam->setValue(result);
}

} // namespace photon
