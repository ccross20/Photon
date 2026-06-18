#include "fixturelistintervalsubsetnode.h"
#include "graph/parameter/fixturelistparameter.h"

namespace photon {

const QByteArray FixtureListIntervalSubsetNode::FixturesInParam = "fixturesIn";
const QByteArray FixtureListIntervalSubsetNode::FixturesOutParam = "fixturesOut";
const QByteArray FixtureListIntervalSubsetNode::OffsetParam = "offset";
const QByteArray FixtureListIntervalSubsetNode::SelectIntervalParam = "select";
const QByteArray FixtureListIntervalSubsetNode::SkipIntervalParam = "skip";


keira::NodeInformation FixtureListIntervalSubsetNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureListIntervalSubsetNode;});
    toReturn.name = "Interval Subset";
    toReturn.nodeId = "photon.fixtures.interval-subset";
    toReturn.categories = {"Fixture List"};

    return toReturn;
}


FixtureListIntervalSubsetNode::FixtureListIntervalSubsetNode() : keira::Node("photon.fixtures.interval-subset")
{
    setName("Interval Subset");
}


void FixtureListIntervalSubsetNode::createParameters()
{


    m_inParam = new FixtureListParameter(FixturesInParam,"Fixtures In",{});
    m_outParam = new FixtureListParameter(FixturesOutParam,"Fixtures Out",{}, keira::AllowMultipleOutput);
    m_offsetParam = new keira::IntegerParameter(OffsetParam, "Offset", 0);
    m_offsetParam->setMinimum(0);
    m_offsetParam->setMaximum(1000);

    m_selectParam = new keira::IntegerParameter(SelectIntervalParam, "Select #", 1);
    m_selectParam->setMinimum(1);
    m_selectParam->setMaximum(1000);


    m_skipParam = new keira::IntegerParameter(SkipIntervalParam, "Skip #", 1);
    m_skipParam->setMinimum(1);
    m_skipParam->setMaximum(1000);

    addParameter(m_inParam);
    addParameter(m_offsetParam);
    addParameter(m_selectParam);
    addParameter(m_skipParam);
    addParameter(m_outParam);

}

void FixtureListIntervalSubsetNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto fixtures = m_inParam->value().value<QVector<FixtureParameterData>>();

    QVector<FixtureParameterData> results;

    int offset = m_offsetParam->value().toInt();
    int selectCount = m_selectParam->value().toInt();
    int skipCount = m_skipParam->value().toInt();

    int counter = 0;
    for(int i = offset; i < fixtures.length(); ++i)
    {
        results.append(fixtures[i]);
        counter++;

        if(counter >= selectCount)
        {
            counter = 0;
            i += skipCount;
        }
    }


    m_outParam->setValue(QVariant::fromValue(results));

}

} // namespace photon
