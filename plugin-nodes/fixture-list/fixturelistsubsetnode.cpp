#include "fixturelistsubsetnode.h"
#include "graph/parameter/fixturelistparameter.h"

namespace photon {

const QByteArray FixtureListSubsetNode::FixturesInParam = "fixturesIn";
const QByteArray FixtureListSubsetNode::OffsetParam = "offset";
const QByteArray FixtureListSubsetNode::SizeParam = "size";
const QByteArray FixtureListSubsetNode::FixturesOutParam = "fixturesOut";

keira::NodeInformation FixtureListSubsetNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureListSubsetNode;});
    toReturn.name = "Subset";
    toReturn.nodeId = "photon.fixtures.subset";
    toReturn.categories = {"Fixture List"};
    return toReturn;
}

FixtureListSubsetNode::FixtureListSubsetNode() : keira::Node("photon.fixtures.subset")
{
    setName("Subset");
}

void FixtureListSubsetNode::createParameters()
{
    m_inParam = new FixtureListParameter(FixturesInParam, "Fixtures In", {});

    m_offsetParam = new keira::IntegerParameter(OffsetParam, "Offset", 0);
    m_offsetParam->setMinimum(-100000);
    m_offsetParam->setMaximum(100000);

    m_sizeParam = new keira::IntegerParameter(SizeParam, "Size", 1);
    m_sizeParam->setMinimum(0);
    m_sizeParam->setMaximum(100000);

    m_outParam = new FixtureListParameter(FixturesOutParam, "Fixtures Out", {}, keira::AllowMultipleOutput);

    addParameter(m_inParam);
    addParameter(m_offsetParam);
    addParameter(m_sizeParam);
    addParameter(m_outParam);
}

void FixtureListSubsetNode::evaluate(keira::EvaluationContext *) const
{
    const auto fixtures = m_inParam->resolvedValue();
    const int count = int(fixtures.size());
    const int size = m_sizeParam->value().toInt();

    QVector<FixtureParameterData> results;
    if(count > 0 && size > 0)
    {
        const int start = ((m_offsetParam->value().toInt() % count) + count) % count;
        const int length = ((size - 1) % count) + 1;   // 1..count, looping
        results.reserve(length);
        for(int i = 0; i < length; ++i)
            results.append(fixtures[(start + i) % count]);
    }

    m_outParam->setValue(QVariant::fromValue(results));
}

} // namespace photon
