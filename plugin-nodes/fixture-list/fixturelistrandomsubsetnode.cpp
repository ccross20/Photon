#include "fixturelistrandomsubsetnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray FixtureListRandomSubsetNode::FixturesInParam = "fixturesIn";
const QByteArray FixtureListRandomSubsetNode::FixturesOutParam = "fixturesOut";
const QByteArray FixtureListRandomSubsetNode::OddsParam = "oddsOfSelection";
const QByteArray FixtureListRandomSubsetNode::SeedParam = "seed";


keira::NodeInformation FixtureListRandomSubsetNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureListRandomSubsetNode;});
    toReturn.name = "Random Subset";
    toReturn.nodeId = "photon.fixtures.random-subset";
    toReturn.categories = {"Fixture List"};

    return toReturn;
}


FixtureListRandomSubsetNode::FixtureListRandomSubsetNode() : keira::Node("photon.fixtures.random-subset")
{
    setName("Random Subset");
}


void FixtureListRandomSubsetNode::createParameters()
{


    m_inParam = new FixtureListParameter(FixturesInParam,"Fixtures In",{});
    m_outParam = new FixtureListParameter(FixturesOutParam,"Fixtures Out",{}, keira::AllowMultipleOutput);
    m_oddsParam = new keira::DecimalParameter(OddsParam, "Odds of Selection",.5);
    m_oddsParam->setMinimum(0);
    m_oddsParam->setMaximum(1.0);
    m_seedParam = new keira::IntegerParameter(SeedParam, "Seed", 0);

    addParameter(m_inParam);
    addParameter(m_oddsParam);
    addParameter(m_seedParam);
    addParameter(m_outParam);

}

void FixtureListRandomSubsetNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto fixtures = m_inParam->value().value<QVector<FixtureParameterData>>();

    QVector<FixtureParameterData> results;

    double odds = m_oddsParam->value().toDouble();
    QRandomGenerator generator(static_cast<uint>(m_seedParam->value().toInt()));

    for(auto fix : fixtures)
    {
        if(generator.generateDouble() < odds)
            results.append(fix);
    }


    m_outParam->setValue(QVariant::fromValue(results));

}

} // namespace photon
