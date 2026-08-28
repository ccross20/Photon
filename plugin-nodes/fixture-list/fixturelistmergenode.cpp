#include <QSet>
#include "fixturelistmergenode.h"
#include "graph/parameter/fixturelistparameter.h"

namespace photon {

const QByteArray FixtureListMergeNode::FixturesAParam = "fixturesA";
const QByteArray FixtureListMergeNode::FixturesBParam = "fixturesB";
const QByteArray FixtureListMergeNode::ModeParam = "mode";
const QByteArray FixtureListMergeNode::FixturesOutParam = "fixturesOut";

namespace {

// The ids in a list, for membership tests. Two entries can carry different
// offsets while referring to the same fixture, so identity is the id alone.
QSet<QByteArray> idsOf(const QVector<FixtureParameterData> &t_fixtures)
{
    QSet<QByteArray> ids;
    ids.reserve(t_fixtures.size());
    for(const auto &fixture : t_fixtures)
        ids.insert(fixture.fixtureId);
    return ids;
}

} // namespace

keira::NodeInformation FixtureListMergeNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureListMergeNode;});
    toReturn.name = "Merge Lists";
    toReturn.nodeId = "photon.fixtures.merge";
    toReturn.categories = {"Fixture List"};

    return toReturn;
}

FixtureListMergeNode::FixtureListMergeNode() : keira::Node("photon.fixtures.merge")
{
    setName("Merge Lists");
}

void FixtureListMergeNode::createParameters()
{
    m_aParam = new FixtureListParameter(FixturesAParam, "Fixtures A", {});
    addParameter(m_aParam);

    m_bParam = new FixtureListParameter(FixturesBParam, "Fixtures B", {});
    addParameter(m_bParam);

    m_modeParam = new keira::OptionParameter(ModeParam, "Mode", {
        "Merge A & B",
        "A without B",
        "B without A",
    }, ModeMerge);
    addParameter(m_modeParam);

    m_outParam = new FixtureListParameter(FixturesOutParam, "Fixtures Out", {}, keira::AllowMultipleOutput);
    addParameter(m_outParam);
}

void FixtureListMergeNode::evaluate(keira::EvaluationContext *) const
{
    // Deliberately value(), not resolvedValue(), unlike the single-input list
    // nodes. resolvedValue() treats an empty unconnected list as "every fixture
    // in the project", which is a sensible default for a node that acts on a
    // list but wrong for a set operation: leaving B unwired would then subtract
    // the whole rig from A, or merge the whole rig into it. Here an unconnected
    // input simply means an empty list.
    const auto listA = m_aParam->value().value<QVector<FixtureParameterData>>();
    const auto listB = m_bParam->value().value<QVector<FixtureParameterData>>();

    QVector<FixtureParameterData> results;

    switch(m_modeParam->value().toInt())
    {
    case ModeAWithoutB:
    {
        const QSet<QByteArray> exclude = idsOf(listB);
        for(const auto &fixture : listA)
            if(!exclude.contains(fixture.fixtureId))
                results.append(fixture);
        break;
    }

    case ModeBWithoutA:
    {
        const QSet<QByteArray> exclude = idsOf(listA);
        for(const auto &fixture : listB)
            if(!exclude.contains(fixture.fixtureId))
                results.append(fixture);
        break;
    }

    case ModeMerge:
    default:
    {
        // A first, then whatever B adds. Deduplicated by id: a fixture landing
        // in the output twice would be evaluated twice downstream (a fixture
        // subgraph would iterate it twice, effects would stack on it), so a
        // fixture present in both lists keeps its entry from A - including A's
        // offset, so an ordering set up upstream survives the merge.
        QSet<QByteArray> seen;
        seen.reserve(listA.size() + listB.size());
        for(const auto &fixture : listA)
            if(!seen.contains(fixture.fixtureId))
            {
                seen.insert(fixture.fixtureId);
                results.append(fixture);
            }
        for(const auto &fixture : listB)
            if(!seen.contains(fixture.fixtureId))
            {
                seen.insert(fixture.fixtureId);
                results.append(fixture);
            }
        break;
    }
    }

    m_outParam->setValue(QVariant::fromValue(results));
}

} // namespace photon
