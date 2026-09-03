#include <cmath>
#include "powernode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray PowerNode::Base = "base";
const QByteArray PowerNode::Exponent = "exponent";
const QByteArray PowerNode::Output = "output";

class PowerNode::Impl
{
public:
    keira::DecimalParameter *baseParam;
    keira::DecimalParameter *exponentParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation PowerNode::info()
{
    keira::NodeInformation toReturn([](){return new PowerNode;});
    toReturn.name = "Power";
    toReturn.nodeId = "photon.math.power";
    toReturn.categories = {"Math"};

    return toReturn;
}

PowerNode::PowerNode() : keira::Node("photon.math.power"), m_impl(new Impl)
{
    setName("Power");
}

PowerNode::~PowerNode()
{
    delete m_impl;
}

void PowerNode::createParameters()
{
    m_impl->baseParam = new keira::DecimalParameter(Base, "Base", 0.0);
    addParameter(m_impl->baseParam);
    m_impl->exponentParam = new keira::DecimalParameter(Exponent, "Exponent", 2.0);
    addParameter(m_impl->exponentParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void PowerNode::evaluate(keira::EvaluationContext *) const
{
    const double result = std::pow(m_impl->baseParam->value().toDouble(),
                                   m_impl->exponentParam->value().toDouble());
    // pow returns NaN for e.g. a negative base with a non-integer exponent -
    // don't let that leak downstream.
    m_impl->outputParam->setValue(std::isfinite(result) ? result : 0.0);
}

} // namespace photon
