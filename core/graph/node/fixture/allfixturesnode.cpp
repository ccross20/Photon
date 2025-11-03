#include "allfixturesnode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
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
    FixtureParameter *fixtureParam;
    QVector<Fixture*> fixtureList;
};

keira::NodeInformation AllFixturesNode::info()
{
    keira::NodeInformation toReturn([](){return new AllFixturesNode;});
    toReturn.name = "All Fixtures";
    toReturn.nodeId = "photon.fixture.all";

    return toReturn;
}


AllFixturesNode::AllFixturesNode() : keira::Node("photon.fixture.all"),m_impl(new Impl)
{
    setName("All Fixtures");
    setIsLoopable(true);
}

AllFixturesNode::~AllFixturesNode()
{
    delete m_impl;
}

void AllFixturesNode::createParameters()
{


    m_impl->fixtureParam = new FixtureParameter(FixtureParam,"Fixture","");
    addParameter(m_impl->fixtureParam);

}

void AllFixturesNode::startLoop()
{
    m_impl->fixtureList = photonApp->project()->fixtures()->fixtures();
}

void AllFixturesNode::endLoop()
{

}

uint AllFixturesNode::loopCount() const
{
    return photonApp->project()->fixtures()->fixtures().count();
}


void AllFixturesNode::evaluate(keira::EvaluationContext *t_context) const
{




    if(!photonApp->project()->fixtures()->fixtures().isEmpty())
    {
        m_impl->fixtureParam->setValue(m_impl->fixtureList[t_context->loopCount]->uniqueId());
    }

}

} // namespace photon
