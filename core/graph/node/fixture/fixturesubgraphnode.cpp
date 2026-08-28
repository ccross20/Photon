#include "fixturesubgraphnode.h"
#include "graph/node/graphcontextnode.h"
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

    m_globalsNode = new GraphContextNode;
    m_globalsNode->configure(GraphContextNode::fixturePorts());
    graph()->addNode(m_globalsNode);
    graph()->drainCommandQueue(); // apply immediately — eval thread not running yet
    graph()->setName("Fixture Graph");
    graph()->setGraphTypeId("fixture");

    // Any edit inside the source subgraph (node parameter change, structural edit)
    // invalidates the cloned pool so evaluate() re-clones with the new values.
    m_dirtyConn = QObject::connect(graph(), &keira::Graph::dirtyStateChanged,
                                   [this]() { m_poolStale = true; });
    // Connecting/disconnecting parameters is a structural edit that does NOT raise
    // dirtyStateChanged, so watch those too — otherwise removing a connection isn't
    // reflected until an unrelated parameter edit re-clones the pool.
    m_connectConn = QObject::connect(graph(), &keira::Graph::parametersWereConnected,
                                     [this](keira::Parameter *, keira::Parameter *) { m_poolStale = true; });
    m_disconnectConn = QObject::connect(graph(), &keira::Graph::parametersWereDisconnected,
                                        [this](keira::Parameter *, keira::Parameter *) { m_poolStale = true; });
}

FixtureSubGraphNode::~FixtureSubGraphNode()
{
    QObject::disconnect(m_dirtyConn);
    QObject::disconnect(m_connectConn);
    QObject::disconnect(m_disconnectConn);
    qDeleteAll(m_subgraphPool);
}

keira::NodeLibrary *FixtureSubGraphNode::nodeLibrary() const
{
    return photonApp->plugins()->nodeLibrary();
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

    m_globalsNode = dynamic_cast<GraphContextNode*>(graph()->findNode("Globals"));

    // Re-run configure so a graph saved before a context port existed picks it
    // up here rather than being stuck with whatever port set it was created
    // with. configure() only adds ports that are missing, so restored ports and
    // their connections are untouched.
    if(m_globalsNode)
        m_globalsNode->configure(GraphContextNode::fixturePorts());

    // Pool must be rebuilt after deserialization
    qDeleteAll(m_subgraphPool);
    m_subgraphPool.clear();
    m_globalsPool.clear();
    m_poolStale = true;
}

void FixtureSubGraphNode::prepForEvaluation()
{
    Node::prepForEvaluation();
    graph()->prepForEvaluation();
}

void FixtureSubGraphNode::markDirty(int t_dirty)
{
    // Deliberately Node::markDirty rather than SubGraphNode::markDirty. The
    // base additionally relays THIS node's dirty down into the authored
    // subgraph, which is what makes the inner graph re-evaluate for the other
    // subgraph kinds - but this node never evaluates the authored graph at
    // all, it only clones from it.
    //
    // With the relay in place, a value arriving on our own Fixtures input
    // during evaluation (i.e. any time a fixture list is wired in, every
    // frame) marked the authored graph dirty, which is indistinguishable from
    // an edit inside the subgraph - so the clone pool was thrown away and
    // rebuilt every frame. That silently reset any per-fixture node state the
    // clones were holding: Scatter Beams' beam positions and headings (making
    // it look frozen, as if Speed were 0), DelayNode's sample buffer, and
    // StopwatchNode's trigger time.
    //
    // Genuine edits inside the subgraph don't need this relay: the edited node
    // marks its own containing graph dirty through Node::markDirty, which is
    // what the pool-stale connection in the constructor listens for. Input
    // values reach the clones without a rebuild too - evaluate() pushes them
    // in via applyInputs() every frame.
    keira::Node::markDirty(t_dirty);
}

// Builds one cloned subgraph per fixture so each parallel evaluation has
// independent node parameter instances and there is no shared write state.
void FixtureSubGraphNode::syncSubgraphPool(int count) const
{
    qDeleteAll(m_subgraphPool);
    m_subgraphPool.clear();
    m_globalsPool.clear();

    if(count == 0)
    {
        m_poolStale = false;
        return;
    }

    graph()->drainCommandQueue();

    QJsonObject subgraphJson;
    graph()->writeToJson(subgraphJson);

    auto *library = photonApp->plugins()->nodeLibrary();

    for(int i = 0; i < count; ++i)
    {
        auto *clone = new keira::Graph;
        clone->readFromJson(subgraphJson, library);

        auto *globals = dynamic_cast<GraphContextNode*>(clone->findNode("Globals"));
        m_subgraphPool.append(clone);
        m_globalsPool.append(globals);
    }

    // Reset the source graph's parameter-dirty bits so the *next* edit re-fires
    // dirtyStateChanged (markDirty short-circuits while a bit is still set).
    graph()->markClean();
    m_poolStale = false;
}

void FixtureSubGraphNode::evaluate(keira::EvaluationContext *t_context) const
{
    if(!m_enabledParam->value().toBool())
        return;

    auto context = static_cast<RoutineEvaluationContext*>(t_context);
    const auto fixtures = m_fixturesParam->resolvedValue();

    if(m_poolStale || m_subgraphPool.size() != fixtures.size())
        syncSubgraphPool(fixtures.size());

    if(m_subgraphPool.isEmpty())
        return;

    // Push values that are constant across all fixtures into each clone: the full
    // fixture list onto the context node, and the exposed graph inputs (relayed
    // from our outer mirror parameters by the base SubGraphNode). Relay the
    // resolved list (not m_fixturesParam->value() directly), so a clone sees
    // the same default-to-all-fixtures fallback this node itself just used.
    const QVariant fixtureListValue = QVariant::fromValue(fixtures);
    for(int i = 0; i < m_subgraphPool.size(); ++i)
    {
        if(m_globalsPool[i])
        {
            m_globalsPool[i]->setValue(GraphContextNode::FixtureListPort, fixtureListValue);
            m_globalsPool[i]->setValue(GraphContextNode::FixtureTotalPort, fixtures.size());
        }
        applyInputs(m_subgraphPool[i]);
    }

    // Parallel fixture evaluation — each fixture uses its own pool clone so node
    // parameter writes never race between threads. The context node fills its
    // per-fixture ports (fixture/index/time) from each fixtureCtx during eval.
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
