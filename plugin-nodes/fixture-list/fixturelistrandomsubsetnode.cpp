#include <algorithm>
#include <cmath>
#include <numeric>
#include <QRandomGenerator>
#include "fixturelistrandomsubsetnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

const QByteArray FixtureListRandomSubsetNode::FixturesInParam = "fixturesIn";
const QByteArray FixtureListRandomSubsetNode::FixturesOutParam = "fixturesOut";
const QByteArray FixtureListRandomSubsetNode::RemainingParam = "fixturesRemaining";
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
    m_outParam = new FixtureListParameter(FixturesOutParam,"Selected",{}, keira::AllowMultipleOutput);
    m_remainingParam = new FixtureListParameter(RemainingParam,"Remaining",{}, keira::AllowMultipleOutput);
    m_oddsParam = new keira::DecimalParameter(OddsParam, "Odds of Selection",.5);
    m_oddsParam->setMinimum(0);
    m_oddsParam->setMaximum(1.0);
    m_seedParam = new keira::IntegerParameter(SeedParam, "Seed", 0);

    addParameter(m_inParam);
    addParameter(m_oddsParam);
    addParameter(m_seedParam);
    addParameter(m_outParam);
    addParameter(m_remainingParam);

}

void FixtureListRandomSubsetNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto fixtures = m_inParam->resolvedValue();

    QVector<FixtureParameterData> selected;
    QVector<FixtureParameterData> remaining;

    // A fixed share rather than an independent roll per fixture: rolling
    // each one let the count drift - 6 fixtures at 0.33 came out empty for a
    // good number of seeds. The count is odds x fixtures, rounded (3 at 0.33
    // -> 1, 6 -> 2), and never 0 while the odds are above 0; the seed only
    // decides which fixtures make up that count.
    const double odds = std::clamp(m_oddsParam->value().toDouble(), 0.0, 1.0);
    const int total = int(fixtures.size());
    int count = int(std::lround(odds * total));
    if(odds > 0.0 && total > 0)
        count = std::max(count, 1);
    count = std::min(count, total);

    // Seeded Fisher-Yates over the indices (Qt's generator, so a seed picks
    // the same fixtures on every machine); the first `count` are selected.
    QVector<int> order(total);
    std::iota(order.begin(), order.end(), 0);
    QRandomGenerator generator(static_cast<uint>(m_seedParam->value().toInt()));
    for(int i = total - 1; i > 0; --i)
        order.swapItemsAt(i, int(generator.bounded(i + 1)));

    QVector<bool> isSelected(total, false);
    for(int i = 0; i < count; ++i)
        isSelected[order[i]] = true;

    // Both outputs keep the original list order, and are exact complements.
    for(int i = 0; i < total; ++i)
    {
        if(isSelected[i])
            selected.append(fixtures[i]);
        else
            remaining.append(fixtures[i]);
    }

    m_outParam->setValue(QVariant::fromValue(selected));
    m_remainingParam->setValue(QVariant::fromValue(remaining));

}

} // namespace photon
