#include <algorithm>
#include "fixturestatenode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/booleanparameter.h"
#include "routine/routineevaluationcontext.h"
#include "state/state.h"
#include "state/stateevaluationcontext.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "project/project.h"
#include "photoncore.h"
#include "surface/viewer/fixturestateeditor.h"

namespace photon {

const QByteArray FixtureStateNode::Fixtures = "fixtures";
const QByteArray FixtureStateNode::Enable = "enable";

keira::NodeInformation FixtureStateNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureStateNode;});
    toReturn.name = "Fixture State";
    toReturn.nodeId = "photon.node.fixture-state";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"bus", "surface"};
    return toReturn;
}

FixtureStateNode::FixtureStateNode() : keira::Node("photon.node.fixture-state")
{
    setName("Fixture State");
    setWidth(160);
    // Always evaluate so the Enable history keeps filling even when nothing
    // upstream changed — the per-fixture offset delay reads from it.
    setIsAlwaysDirty(true);
    m_state = new State;
}

FixtureStateNode::~FixtureStateNode()
{
    delete m_state;
}

State *FixtureStateNode::state() const
{
    return m_state;
}

void FixtureStateNode::createParameters()
{
    m_fixturesParam = new FixtureListParameter(Fixtures, "Fixtures", QVector<FixtureParameterData>(),
                                               keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_fixturesParam);

    m_enableParam = new keira::BooleanParameter(Enable, "Enable", true,
                                                keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_enableParam);
}

// Nearest-neighbour lookup of the Enable value at a past time.
static bool enabledAt(const std::deque<FixtureStateNode::EnableSample> &t_history, double t_time)
{
    if(t_history.empty())
        return true;
    if(t_history.front().time >= t_time)
        return t_history.front().enabled; // not enough history yet

    int lo = 0, hi = static_cast<int>(t_history.size()) - 1;
    while(lo + 1 < hi)
    {
        const int mid = (lo + hi) / 2;
        if(t_history[mid].time <= t_time)
            lo = mid;
        else
            hi = mid;
    }
    return t_history[lo].enabled;
}

void FixtureStateNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto context = static_cast<RoutineEvaluationContext*>(t_context);
    const auto fixtures = m_fixturesParam->value().value<QVector<FixtureParameterData>>();

    const double now = context->globalTime;

    // Record this frame's Enable value.
    m_enableHistory.push_back({now, m_enableParam->value().toBool()});

    // Prune history older than the largest offset we might look back to.
    double maxOffset = 0.0;
    for(const auto &fixtureData : fixtures)
        maxOffset = std::max(maxOffset, fixtureData.offset);
    const double cutoff = now - maxOffset - 1.0;
    while(m_enableHistory.size() > 1 && m_enableHistory.front().time < cutoff)
        m_enableHistory.pop_front();

    for(const auto &fixtureData : fixtures)
    {
        auto *fixture = photonApp->project()->fixtures()->fixtureWithId(fixtureData.fixtureId);
        if(!fixture)
            continue;

        // Enable is delayed per fixture, so toggling it staggers across the rig.
        if(!enabledAt(m_enableHistory, now - fixtureData.offset))
            continue;

        StateEvaluationContext local(context->dmxMatrix);
        local.fixture      = fixture;
        local.strength     = context->strength;
        local.globalTime   = context->globalTime;
        local.relativeTime = context->relativeTime;

        // Seed channel values from the capabilities' stored values, then run.
        // (Exposed-parameter overrides will be injected here in a later step.)
        local.channelValues.clear();
        m_state->initializeValues(local);
        m_state->evaluate(local);
    }
}

QWidget *FixtureStateNode::createCustomWidget(keira::NodeEditor *)
{
    return new FixtureStateEditor(m_state);
}

void FixtureStateNode::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    keira::Node::readFromJson(t_json, t_library);

    if(t_json.contains("state"))
    {
        LoadContext context;
        delete m_state;
        m_state = new State;
        m_state->readFromJson(t_json.value("state").toObject(), context);
    }
}

void FixtureStateNode::writeToJson(QJsonObject &t_json) const
{
    keira::Node::writeToJson(t_json);

    QJsonObject stateObj;
    m_state->writeToJson(stateObj);
    t_json.insert("state", stateObj);
}

} // namespace photon
