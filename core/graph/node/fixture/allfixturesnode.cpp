#include "allfixturesnode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {


const QByteArray AllFixturesNode::FixtureParam = "fixture";

class AllFixturesNode::Impl
{
public:
    FixtureListParameter *fixtureParam;
    QVector<Fixture*> fixtureList;
};

keira::NodeInformation AllFixturesNode::info()
{
    keira::NodeInformation toReturn([](){return new AllFixturesNode;});
    toReturn.name = "All Fixtures";
    toReturn.nodeId = "photon.fixture.all";
    toReturn.categories = {"Fixture List"};

    return toReturn;
}


AllFixturesNode::AllFixturesNode() : keira::Node("photon.fixture.all"),m_impl(new Impl)
{
    setName("All Fixtures");
}

AllFixturesNode::~AllFixturesNode()
{
    delete m_impl;
}

void AllFixturesNode::createParameters()
{


    m_impl->fixtureParam = new FixtureListParameter(FixtureParam,"Fixture",{});
    addParameter(m_impl->fixtureParam);

}

void AllFixturesNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto allFixtures = photonApp->project()->fixtures()->fixtures();
    QVector<FixtureParameterData> dataList;

    for(auto fixture : allFixtures)
    {
        dataList.append(fixture);
    }

    m_impl->fixtureParam->setValue(QVariant::fromValue(dataList));

}

} // namespace photon
