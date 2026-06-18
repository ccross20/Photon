#include "linearfalloffnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {


keira::NodeInformation LinearFalloffNode::info()
{
    keira::NodeInformation toReturn([](){return new LinearFalloffNode;});
    toReturn.name = "Linear Falloff";
    toReturn.nodeId = "photon.falloff.linear";
    toReturn.categories = {"Falloff"};

    return toReturn;
}


LinearFalloffNode::LinearFalloffNode() : keira::Node("photon.falloff.linear") {}


void LinearFalloffNode::createParameters()
{


    m_inParam = new FixtureListParameter("in","Fixtures In",{});
    m_outParam = new FixtureListParameter("out","Fixtures Out",{}, keira::AllowMultipleOutput);
    m_totalOffsetParam = new keira::DecimalParameter("offset", "Max Falloff", 1.0);
    m_totalOffsetParam->setMinimum(0);
    m_totalOffsetParam->setMaximum(10.0);
    m_reverseParam = new keira::BooleanParameter("reverse", "Reverse", 0);

    addParameter(m_inParam);
    addParameter(m_totalOffsetParam);
    addParameter(m_reverseParam);
    addParameter(m_outParam);

}

void LinearFalloffNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto fixtures = m_inParam->value().value<QVector<FixtureParameterData>>();

    double offset = m_totalOffsetParam->value().toDouble();
    bool reverse = m_reverseParam->value().toBool();

    double increment = offset / (fixtures.length()-1);
    double currentAmount = 0;

    if(reverse)
    {
        increment = -increment;
        currentAmount = offset;
    }

    for(auto &fix : fixtures)
    {
        fix.offset = currentAmount;
        currentAmount += increment;
    }


    m_outParam->setValue(QVariant::fromValue(fixtures));

}

} // namespace photon
