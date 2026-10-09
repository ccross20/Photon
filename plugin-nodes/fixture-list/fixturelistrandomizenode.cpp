#include <QRandomGenerator>
#include "fixturelistrandomizenode.h"
#include "graph/parameter/fixturelistparameter.h"

namespace photon {

const QByteArray FixtureListRandomizeNode::FixturesInParam = "fixturesIn";
const QByteArray FixtureListRandomizeNode::SeedParam = "seed";
const QByteArray FixtureListRandomizeNode::FixturesOutParam = "fixturesOut";

keira::NodeInformation FixtureListRandomizeNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureListRandomizeNode;});
    toReturn.name = "Randomize Order";
    toReturn.nodeId = "photon.fixtures.randomize";
    toReturn.categories = {"Fixture List"};
    return toReturn;
}

FixtureListRandomizeNode::FixtureListRandomizeNode() : keira::Node("photon.fixtures.randomize")
{
    setName("Randomize Order");
}

void FixtureListRandomizeNode::createParameters()
{
    m_inParam = new FixtureListParameter(FixturesInParam, "Fixtures In", {});
    m_seedParam = new keira::IntegerParameter(SeedParam, "Seed", 0);
    m_outParam = new FixtureListParameter(FixturesOutParam, "Fixtures Out", {}, keira::AllowMultipleOutput);

    addParameter(m_inParam);
    addParameter(m_seedParam);
    addParameter(m_outParam);
}

void FixtureListRandomizeNode::evaluate(keira::EvaluationContext *) const
{
    auto fixtures = m_inParam->resolvedValue();

    // Fisher-Yates with Qt's generator rather than std::shuffle, whose
    // algorithm differs between standard libraries - a seed should give the
    // same order on every machine.
    QRandomGenerator generator(static_cast<quint32>(m_seedParam->value().toInt()));
    for(int i = int(fixtures.size()) - 1; i > 0; --i)
        fixtures.swapItemsAt(i, int(generator.bounded(i + 1)));

    m_outParam->setValue(QVariant::fromValue(fixtures));
}

} // namespace photon
