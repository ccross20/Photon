#include <algorithm>
#include "fixturelistsortnode.h"
#include "graph/parameter/fixturelistparameter.h"

namespace photon {

const QByteArray FixtureListSortNode::FixturesInParam = "fixturesIn";
const QByteArray FixtureListSortNode::OrderParam = "order";
const QByteArray FixtureListSortNode::FixturesOutParam = "fixturesOut";

keira::NodeInformation FixtureListSortNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureListSortNode;});
    toReturn.name = "Sort by Offset";
    toReturn.nodeId = "photon.fixtures.sort-by-offset";
    toReturn.categories = {"Fixture List"};
    return toReturn;
}

FixtureListSortNode::FixtureListSortNode() : keira::Node("photon.fixtures.sort-by-offset")
{
    setName("Sort by Offset");
}

void FixtureListSortNode::createParameters()
{
    m_inParam = new FixtureListParameter(FixturesInParam, "Fixtures In", {});
    m_orderParam = new keira::OptionParameter(OrderParam, "Order", {"Ascending", "Descending"}, OrderAscending);
    m_outParam = new FixtureListParameter(FixturesOutParam, "Fixtures Out", {}, keira::AllowMultipleOutput);

    addParameter(m_inParam);
    addParameter(m_orderParam);
    addParameter(m_outParam);
}

void FixtureListSortNode::evaluate(keira::EvaluationContext *) const
{
    auto fixtures = m_inParam->resolvedValue();

    // Stable, so fixtures sharing an offset keep their incoming order.
    if(m_orderParam->value().toInt() == OrderDescending)
        std::stable_sort(fixtures.begin(), fixtures.end(),
                         [](const FixtureParameterData &a, const FixtureParameterData &b){ return a.offset > b.offset; });
    else
        std::stable_sort(fixtures.begin(), fixtures.end(),
                         [](const FixtureParameterData &a, const FixtureParameterData &b){ return a.offset < b.offset; });

    m_outParam->setValue(QVariant::fromValue(fixtures));
}

} // namespace photon
