#include "fixturetreeselection.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {


const QByteArray FixtureTreeSelection::Fixture = "fixture";

class FixtureTreeSelection::Impl
{
public:
    FixtureParameter *fixtureParam;
};

keira::NodeInformation FixtureTreeSelection::info()
{
    keira::NodeInformation toReturn([](){return new FixtureTreeSelection;});
    toReturn.name = "Fixture Tree Selection";
    toReturn.nodeId = "photon.fixture.tree-selection";

    return toReturn;
}


FixtureTreeSelection::FixtureTreeSelection() : keira::Node("photon.fixture.tree-selection"),m_impl(new Impl)
{
    setName("Fixture Tree Selection");
}

FixtureTreeSelection::~FixtureTreeSelection()
{
    delete m_impl;
}

void FixtureTreeSelection::createParameters()
{


    m_impl->fixtureParam = new FixtureParameter(Fixture,"Fixture","");
    addParameter(m_impl->fixtureParam);

}

void FixtureTreeSelection::evaluate(keira::EvaluationContext *) const
{


    QByteArray fixtureId = m_impl->fixtureParam->value().toByteArray();
    fixtureId = "test";


}

} // namespace photon
