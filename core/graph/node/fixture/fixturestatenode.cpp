#include <algorithm>
#include "fixturestatenode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/anyparameter.h"
#include "model/graph.h"
#include "routine/routineevaluationcontext.h"
#include "state/state.h"
#include "state/statecapability.h"
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
    toReturn.graphs = QByteArrayList{"bus", "surface", "dmx-subgraph"};
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
    qDeleteAll(m_retiredParams);
}

bool FixtureStateNode::isChannelExposed(StateCapability *t_cap, int t_index) const
{
    return findParameter(t_cap->channelId(t_index)) != nullptr;
}

void FixtureStateNode::setChannelExposed(StateCapability *t_cap, int t_index, bool t_exposed)
{
    const QByteArray channelId = t_cap->channelId(t_index);
    keira::Parameter *existing = findParameter(channelId);

    if(t_exposed)
    {
        if(existing)
            return;
        auto *param = new keira::AnyParameter(channelId, t_cap->name(),
                                              keira::AllowMultipleOutput | keira::AllowSingleInput);
        // Apply the parameter change on the eval thread so it never races the
        // evaluator iterating this node's parameters.
        auto apply = [this, param]() { addParameter(param); portsChanged(); };
        if(graph())
            graph()->runCommand(apply);
        else
            apply();
    }
    else
    {
        if(!existing)
            return;

        if(graph())
        {
            // Queue the disconnects first, then the removal, so they drain in
            // order. Retire (don't delete) the parameter so the eval thread
            // never touches a freed pointer.
            const auto inputs = existing->inputParameters();
            for(auto *in : inputs)
                graph()->disconnectParameters(in, existing);
            const auto outputs = existing->outputParameters();
            for(auto *out : outputs)
                graph()->disconnectParameters(existing, out);

            graph()->runCommand([this, existing, channelId]() {
                removeParameter(existing);
                m_inputHistory.remove(channelId);
                m_retiredParams.append(existing);
                portsChanged();
            });
        }
        else
        {
            removeParameter(existing);
            m_inputHistory.remove(channelId);
            m_retiredParams.append(existing);
        }
    }
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

// Index of the last sample at or before t_time (nearest-neighbour before).
template<typename Sample>
static int sampleIndexAt(const std::deque<Sample> &t_history, double t_time)
{
    int lo = 0, hi = static_cast<int>(t_history.size()) - 1;
    while(lo + 1 < hi)
    {
        const int mid = (lo + hi) / 2;
        if(t_history[mid].time <= t_time)
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}

static bool isNumeric(const QVariant &t_value)
{
    switch(t_value.typeId())
    {
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return true;
    default:
        return false;
    }
}

// Value of an exposed input at a past time: linearly interpolated for numeric
// values, nearest-neighbour for everything else (colors, bools, ...).
static QVariant sampleValueAt(const std::deque<FixtureStateNode::ValueSample> &t_history, double t_time)
{
    if(t_history.empty())
        return QVariant();
    if(t_history.front().time >= t_time)
        return t_history.front().value;

    const int lo = sampleIndexAt(t_history, t_time);
    if(lo + 1 >= static_cast<int>(t_history.size()))
        return t_history[lo].value;

    const auto &a = t_history[lo];
    const auto &b = t_history[lo + 1];
    if(isNumeric(a.value) && isNumeric(b.value))
    {
        const double span = b.time - a.time;
        if(span < 1e-9)
            return a.value;
        const double frac = (t_time - a.time) / span;
        return a.value.toDouble() + frac * (b.value.toDouble() - a.value.toDouble());
    }
    return a.value;
}

// Nearest-neighbour lookup of the Enable value at a past time.
static bool enabledAt(const std::deque<FixtureStateNode::EnableSample> &t_history, double t_time)
{
    if(t_history.empty())
        return true;
    if(t_history.front().time >= t_time)
        return t_history.front().enabled; // not enough history yet
    return t_history[sampleIndexAt(t_history, t_time)].enabled;
}

void FixtureStateNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto context = static_cast<RoutineEvaluationContext*>(t_context);
    const auto fixtures = m_fixturesParam->value().value<QVector<FixtureParameterData>>();

    const double now = context->globalTime;

    // Prune histories older than the largest offset we might look back to.
    double maxOffset = 0.0;
    for(const auto &fixtureData : fixtures)
        maxOffset = std::max(maxOffset, fixtureData.offset);
    const double cutoff = now - maxOffset - 1.0;

    // Record this frame's Enable value.
    m_enableHistory.push_back({now, m_enableParam->value().toBool()});
    while(m_enableHistory.size() > 1 && m_enableHistory.front().time < cutoff)
        m_enableHistory.pop_front();

    // Record each connected exposed input's value.
    for(auto *param : parameters())
    {
        if(param == m_fixturesParam || param == m_enableParam || !param->hasInput())
            continue;
        auto &history = m_inputHistory[param->id()];
        history.push_back({now, param->value()});
        while(history.size() > 1 && history.front().time < cutoff)
            history.pop_front();
    }

    for(const auto &fixtureData : fixtures)
    {
        auto *fixture = photonApp->project()->fixtures()->fixtureWithId(fixtureData.fixtureId);
        if(!fixture)
            continue;

        const double delayedTime = now - fixtureData.offset;

        // Enable is delayed per fixture, so toggling it staggers across the rig.
        if(!enabledAt(m_enableHistory, delayedTime))
            continue;

        StateEvaluationContext local(context->dmxMatrix);
        local.fixture      = fixture;
        local.strength     = context->strength;
        local.globalTime   = context->globalTime;
        local.relativeTime = context->relativeTime;

        // Seed from the capabilities' static values, then override any exposed
        // (connected) channel with its per-fixture offset-delayed driven value.
        local.channelValues.clear();
        m_state->initializeValues(local);

        for(auto *param : parameters())
        {
            if(param == m_fixturesParam || param == m_enableParam || !param->hasInput())
                continue;
            const auto &history = m_inputHistory[param->id()];
            if(history.empty())
                continue;
            local.channelValues[param->id()] = sampleValueAt(history, delayedTime);
        }

        m_state->evaluate(local);
    }
}

QWidget *FixtureStateNode::createCustomWidget(keira::NodeEditor *)
{
    return new FixtureStateEditor(this);
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
