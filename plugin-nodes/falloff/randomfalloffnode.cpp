#include <QRandomGenerator>
#include "randomfalloffnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class RandomFalloffNode::Impl{
public:
    QRandomGenerator m_random;
};


keira::NodeInformation RandomFalloffNode::info()
{
    keira::NodeInformation toReturn([](){return new RandomFalloffNode;});
    toReturn.name = "Random Falloff";
    toReturn.nodeId = "photon.falloff.random";
    toReturn.categories = {"Falloff"};

    return toReturn;
}


RandomFalloffNode::RandomFalloffNode() : keira::Node("photon.falloff.random"),m_impl(new Impl) {}

RandomFalloffNode::~RandomFalloffNode()
{
    delete m_impl;
}

void RandomFalloffNode::createParameters()
{


    m_inParam = new FixtureListParameter("in","Fixtures In",{});
    m_outParam = new FixtureListParameter("out","Fixtures Out",{}, keira::AllowMultipleOutput);
    m_totalOffsetParam = new keira::DecimalParameter("offset", "Max Falloff", 1.0);
    m_totalOffsetParam->setMinimum(0);
    m_totalOffsetParam->setMaximum(10.0);
    m_seedParam = new keira::IntegerParameter("seed", "Seed", 1);

    m_equalDistributionParam = new keira::BooleanParameter("equal", "Equal Distribution", true);

    addParameter(m_inParam);
    addParameter(m_totalOffsetParam);
    addParameter(m_seedParam);
    addParameter(m_equalDistributionParam);
    addParameter(m_outParam);

}

void RandomFalloffNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto fixtures = m_inParam->value().value<QVector<FixtureParameterData>>();

    double offset = m_totalOffsetParam->value().toDouble();
    int seed = m_seedParam->value().toInt();

    double increment = offset / (fixtures.length()-1);
    double currentAmount = 0;

    m_impl->m_random.seed(seed);

    QVector<double> offsets(fixtures.length());

    if(m_equalDistributionParam->value().toBool())
    {
        for(auto it = offsets.begin(); it != offsets.end(); ++it)
        {
            (*it) = currentAmount;
            currentAmount += increment;
        }

        std::shuffle(offsets.begin(), offsets.end(), m_impl->m_random);
    }
    else
    {
        for(auto it = offsets.begin(); it != offsets.end(); ++it)
        {
            (*it) = m_impl->m_random.generateDouble() * offset;
        }
    }

    auto fixIt = fixtures.begin();
    for(auto it = offsets.begin(); it != offsets.end(); ++it, ++fixIt)
    {
        (*fixIt).offset = *it;
    }

    m_outParam->setValue(QVariant::fromValue(fixtures));

}

} // namespace photon

