#include <algorithm>
#include "clampnode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray ClampNode::Input = "input";
const QByteArray ClampNode::MinInput = "min";
const QByteArray ClampNode::MaxInput = "max";
const QByteArray ClampNode::Output = "output";

class ClampNode::Impl
{
public:
    keira::DecimalParameter *inputParam;
    keira::DecimalParameter *minParam;
    keira::DecimalParameter *maxParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation ClampNode::info()
{
    keira::NodeInformation toReturn([](){return new ClampNode;});
    toReturn.name = "Clamp";
    toReturn.nodeId = "photon.math.clamp";
    toReturn.categories = {"Math"};

    return toReturn;
}

ClampNode::ClampNode() : keira::Node("photon.math.clamp"), m_impl(new Impl)
{
    setName("Clamp");
}

ClampNode::~ClampNode()
{
    delete m_impl;
}

void ClampNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 0.0);
    addParameter(m_impl->inputParam);
    m_impl->minParam = new keira::DecimalParameter(MinInput, "Min", 0.0);
    addParameter(m_impl->minParam);
    m_impl->maxParam = new keira::DecimalParameter(MaxInput, "Max", 1.0);
    addParameter(m_impl->maxParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void ClampNode::evaluate(keira::EvaluationContext *) const
{
    const double in = m_impl->inputParam->value().toDouble();
    double lo = m_impl->minParam->value().toDouble();
    double hi = m_impl->maxParam->value().toDouble();
    if(lo > hi)
        std::swap(lo, hi);

    m_impl->outputParam->setValue(std::clamp(in, lo, hi));
}

} // namespace photon
