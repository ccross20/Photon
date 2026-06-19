#include "fixturesubgraphnode.h"
#include "fixtureglobalsnode.h"
#include "model/graph.h"
#include "surface/surfacegraph.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "plugin/pluginfactory.h"
#include "routine/routineevaluationcontext.h"
#include <QtConcurrent>
#include <numeric>

namespace photon {

const QByteArray FixtureSubGraphNode::Fixtures = "fixtures";
const QByteArray FixtureSubGraphNode::Enabled = "enabled";
const QByteArray FixtureSubGraphNode::FixtureSubGraphId = "fixture";

keira::NodeInformation FixtureSubGraphNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureSubGraphNode;});
    toReturn.name = "Fixture Graph";
    toReturn.nodeId = "photon.node.fixture-graph";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"bus","surface"};

    return toReturn;
}

FixtureSubGraphNode::FixtureSubGraphNode() : keira::SubGraphNode("photon.node.fixture-graph") {
    setName("Fixture Graph");

    m_globalsNode = new FixtureGlobalsNode;
    m_globalsNode->createParameters();
    graph()->addNode(m_globalsNode);
    graph()->drainCommandQueue(); // apply immediately — eval thread not running yet
    graph()->setName("Fixture Graph");
    graph()->setGraphTypeId("fixture");
}

FixtureSubGraphNode::~FixtureSubGraphNode()
{
    qDeleteAll(m_subgraphPool);
}

void FixtureSubGraphNode::createParameters()
{
    m_fixturesParam = new FixtureListParameter(FixtureSubGraphNode::Fixtures, "Fixtures",
                                               QVector<FixtureParameterData>(),
                                               keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_fixturesParam);

    m_enabledParam = new keira::BooleanParameter(Enabled, "Enabled", true);
    addParameter(m_enabledParam);

    m_priortyParam = new keira::IntegerParameter("priority", "Priority", 0);
    addParameter(m_priortyParam);
}

void FixtureSubGraphNode::parameterWasAdded(keira::Parameter *t_param)
{
    if(m_fixturesParam == t_param || m_enabledParam == t_param || t_param == m_priortyParam)
        return;

    auto clonedParam = t_param->clone(photonApp->plugins()->nodeLibrary());
    clonedParam->setConnectionOptions(keira::AllowMultipleOutput);
    m_globalsNode->addParameter(clonedParam);
    graph()->drainCommandQueue();
    m_passThroughParams.append(clonedParam);
    m_globalsParams.append(t_param);

    // Pool is stale — rebuild on next evaluate()
    qDeleteAll(m_subgraphPool);
    m_subgraphPool.clear();
    m_globalsPool.clear();
}

void FixtureSubGraphNode::parameterWasRemoved(keira::Parameter *)
{
}

void FixtureSubGraphNode::parameterWasModified(keira::Parameter *t_param)
{
    if(t_param == m_priortyParam)
    {
        setPriority(m_priortyParam->value().toInt());
        markDirty(keira::Dirty_Priority);
    }
}

void FixtureSubGraphNode::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    graph()->removeNode(m_globalsNode);
    graph()->drainCommandQueue(); // apply before deleting the pointer
    delete m_globalsNode;

    keira::SubGraphNode::readFromJson(t_json, t_library);

    m_globalsNode = dynamic_cast<FixtureGlobalsNode*>(graph()->findNode("Globals"));

    for(auto param : parameters())
    {
        if(m_fixturesParam == param || param == m_priortyParam)
            continue;

        auto nodeParam = m_globalsNode->findParameter(param->id());
        if(nodeParam)
        {
            m_globalsParams.append(param);
            m_passThroughParams.append(nodeParam);
        }
        else
            qWarning() << "Could not relink parameter: " << param->id();
    }

    // Pool must be rebuilt after deserialization
    qDeleteAll(m_subgraphPool);
    m_subgraphPool.clear();
    m_globalsPool.clear();
}

void FixtureSubGraphNode::prepForEvaluation()
{
    Node::prepForEvaluation();
    graph()->prepForEvaluation();
}

// Builds one cloned subgraph per fixture so each parallel evaluation has
// independent node parameter instances and there is no shared write state.
void FixtureSubGraphNode::syncSubgraphPool(int count) const
{
    qDeleteAll(m_subgraphPool);
    m_subgraphPool.clear();
    m_globalsPool.clear();

    if(count == 0)
        return;

    graph()->drainCommandQueue();

    QJsonObject subgraphJson;
    graph()->writeToJson(subgraphJson);

    auto *library = photonApp->plugins()->nodeLibrary();

    for(int i = 0; i < count; ++i)
    {
        auto *clone = new keira::Graph;
        clone->readFromJson(subgraphJson, library);

        auto *globals = dynamic_cast<FixtureGlobalsNode*>(clone->findNode("Globals"));
        m_subgraphPool.append(clone);
        m_globalsPool.append(globals);
    }
}

void FixtureSubGraphNode::evaluate(keira::EvaluationContext *t_context) const
{
    if(!m_enabledParam->value().toBool())
        return;

    auto context = static_cast<RoutineEvaluationContext*>(t_context);
    const auto fixtures = m_fixturesParam->value().value<QVector<FixtureParameterData>>();

    if(m_subgraphPool.size() != fixtures.size())
        syncSubgraphPool(fixtures.size());

    if(m_subgraphPool.isEmpty())
        return;

    // Push values that are constant across all fixtures into each clone's globals.
    const QVariant fixtureListValue = m_fixturesParam->value();
    for(int i = 0; i < m_subgraphPool.size(); ++i)
    {
        if(!m_globalsPool[i])
            continue;
        m_globalsPool[i]->setValue(FixtureGlobalsNode::FixtureListParam, fixtureListValue);

        for(int p = 0; p < m_passThroughParams.size(); ++p)
        {
            auto *cloneParam = m_globalsPool[i]->findParameter(m_passThroughParams[p]->id());
            if(cloneParam)
                cloneParam->setValue(m_globalsParams[p]->value());
        }
    }

    // Parallel fixture evaluation — each fixture uses its own pool clone so
    // node parameter writes never race between threads.
    QVector<int> indices(fixtures.size());
    std::iota(indices.begin(), indices.end(), 0);

    QtConcurrent::blockingMap(indices, [&](int i)
    {
        const auto &fixtureData = fixtures[i];
        auto *fix = photonApp->project()->fixtures()->fixtureWithId(fixtureData.fixtureId);
        if(!fix)
            return;

        RoutineEvaluationContext fixtureCtx(*context);
        fixtureCtx.fixture      = fix;
        fixtureCtx.fixtureIndex = i;
        fixtureCtx.timeOffset   = fixtureData.offset;
        fixtureCtx.relativeTime = context->globalTime + fixtureData.offset;

        m_subgraphPool[i]->evaluateAll(&fixtureCtx);
    });
}

} // namespace photon
