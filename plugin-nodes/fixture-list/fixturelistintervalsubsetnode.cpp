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
    // Wraps around the list (see evaluate()), so negative offsets just run
    // the pattern the other way.
    m_offsetParam->setMinimum(-1000);
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

    auto fixtures = m_inParam->resolvedValue();

    QVector<FixtureParameterData> results;

    int offset = m_offsetParam->value().toInt();
    int selectCount = m_selectParam->value().toInt();
    int skipCount = m_skipParam->value().toInt();

    // The select/skip pattern starts at the offset fixture and wraps past
    // the end back to the start, so every fixture is covered whatever the
    // offset, and offsets past the list's length loop around (with 8
    // fixtures, offset 10 is offset 2). Stepping the offset therefore
    // chases the pattern round the list. Selected fixtures keep their
    // original list order.
    const int count = int(fixtures.length());
    const int period = selectCount + skipCount;
    if(count > 0 && period > 0)
    {
        const int start = ((offset % count) + count) % count;
        QVector<bool> selected(count, false);
        for(int step = 0; step < count; ++step)
        {
            if(step % period < selectCount)
                selected[(start + step) % count] = true;
        }
        for(int i = 0; i < count; ++i)
        {
            if(selected[i])
                results.append(fixtures[i]);
        }
    }


    m_outParam->setValue(QVariant::fromValue(results));

}

} // namespace photon
