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
    toReturn.graphs = QByteArrayList{"bus", "surface", "dmx-subgraph", "routine", "fixture"};
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

QString FixtureStateNode::portNameFor(StateCapability *t_cap, int t_index)
{
    const auto channels = t_cap->availableChannels();
    if(t_index < 0 || t_index >= channels.size())
        return t_cap->name();

    const QString channelName = channels[t_index].name;
    // Naming a port after the capability alone (as this used to) gives every
    // channel of a capability the same label - three ports all called "Pan".
    // Qualifying the channel with the capability keeps them apart from each
    // other AND from same-named channels of other capabilities, since several
    // states have a channel called "Name". Where the two already match, the
    // capability name alone reads better than "Color Color".
    if(channelName.isEmpty())
        return t_cap->name();
    if(channelName.compare(t_cap->name(), Qt::CaseInsensitive) == 0)
        return t_cap->name();
    return t_cap->name() + " " + channelName;
}

void FixtureStateNode::setChannelExposed(StateCapability *t_cap, int t_index, bool t_exposed)
{
    const QByteArray channelId = t_cap->channelId(t_index);
    keira::Parameter *existing = findParameter(channelId);

    if(t_exposed)
    {
        if(existing)
            return;
        auto *param = new keira::AnyParameter(channelId, portNameFor(t_cap, t_index), keira::AllowSingleInput);
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

QVector<Fixture*> FixtureStateNode::resolvedFixtures() const
{
    QVector<Fixture*> results;
    const auto fixtures = m_fixturesParam->resolvedValue();
    for(const auto &fixtureData : fixtures)
    {
        auto *fixture = photonApp->project()->fixtures()->fixtureWithId(fixtureData.fixtureId);
        if(fixture)
            results.append(fixture);
    }
    return results;
}

void FixtureStateNode::createParameters()
{
    m_fixturesParam = new FixtureListParameter(Fixtures, "Fixtures", QVector<FixtureParameterData>(),
                                                keira::AllowSingleInput);
    addParameter(m_fixturesParam);

    m_enableParam = new keira::BooleanParameter(Enable, "Enable", true,
                                                keira::AllowSingleInput);
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

void FixtureStateNode::applyToFixture(RoutineEvaluationContext &t_context, Fixture *t_fixture,
                                      const QHash<QByteArray, QVariant> &t_overrides) const
{
    StateEvaluationContext local(t_context.dmxMatrix);
    local.fixture      = t_fixture;
    local.strength     = t_context.strength;
    local.globalTime   = t_context.globalTime;
    local.relativeTime = t_context.relativeTime;

    // Seed from the capabilities' static values, then let any exposed
    // (connected) channel override its entry.
    local.channelValues.clear();
    m_state->initializeValues(local);

    for(auto it = t_overrides.cbegin(); it != t_overrides.cend(); ++it)
        local.channelValues[it.key()] = it.value();

    m_state->evaluate(local);
}

void FixtureStateNode::evaluate(keira::EvaluationContext *t_context) const
{
    // Guarded: a graph opened directly in the node editor is live-ticked with a
    // plain keira::EvaluationContext, not a routine one (GraphContextNode
    // guards the same way for the same reason).
    auto context = dynamic_cast<RoutineEvaluationContext*>(t_context);
    if(!context)
        return;

    // Per-fixture mode. Inside a fixture subgraph the enclosing
    // FixtureSubGraphNode has already chosen this fixture, already applied its
    // time offset to relativeTime, and is evaluating one clone per fixture in
    // parallel. So: apply to that fixture only.
    //
    // Doing the standalone thing here instead would be actively wrong, not just
    // redundant - each of the N clones would write all N fixtures (N² writes),
    // and that breaks the invariant the parallel loop relies on, that fixture
    // evaluations touch non-overlapping channels. DMXMatrix::setValuePercent is
    // a read-modify-write, so with strength below 1 the output tears rather
    // than merely repeating work.
    //
    // The value histories are skipped too: they exist to stagger ONE node
    // across MANY fixtures, and here the subgraph has already done the
    // staggering. Every upstream node in this clone has just been evaluated for
    // this fixture, so the exposed parameters already hold the right values and
    // reading them directly is the only way not to double-count the offset.
    if(context->fixture)
    {
        // The Fixtures port stays useful here as an optional filter. Only
        // consult it when it actually holds a selection - resolvedValue()'s
        // "unwired means everything" fallback would make the filter a no-op
        // while costing an all-fixtures build per clone per frame.
        if(m_fixturesParam->hasInput()
           || !m_fixturesParam->value().value<QVector<FixtureParameterData>>().isEmpty())
        {
            const auto allowed = m_fixturesParam->resolvedValue();
            bool found = false;
            for(const auto &fixtureData : allowed)
            {
                if(fixtureData.fixtureId == context->fixture->uniqueId())
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                return;
        }

        if(!m_enableParam->value().toBool())
            return;

        QHash<QByteArray, QVariant> overrides;
        for(auto *param : parameters())
        {
            if(param == m_fixturesParam || param == m_enableParam || !param->hasInput())
                continue;
            overrides.insert(param->id(), param->value());
        }

        applyToFixture(*context, context->fixture, overrides);
        return;
    }

    // Standalone mode: this node owns the fixture iteration and the staggering.
    const auto fixtures = m_fixturesParam->resolvedValue();

    const double now = context->globalTime;

    // A backward jump in time (scrub, rewind, loop replay) breaks the assumption
    // below that samples are appended in increasing time order: sampleValueAt/
    // enabledAt binary-search the deque assuming ascending time, so pushing an
    // older timestamp onto the back of a deque still full of samples from the end
    // of the previous playthrough corrupts every lookup until those stale entries
    // age out - which, since nothing here is currently increasing `now`, is never.
    // Simplest correct fix: a time jump invalidates the lookback window outright,
    // so just wipe it and let it rebuild from here.
    if(!m_enableHistory.empty() && now < m_enableHistory.back().time - 1e-6)
    {
        m_enableHistory.clear();
        m_inputHistory.clear();
    }

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

        // Each exposed channel contributes its per-fixture offset-delayed value.
        QHash<QByteArray, QVariant> overrides;
        for(auto *param : parameters())
        {
            if(param == m_fixturesParam || param == m_enableParam || !param->hasInput())
                continue;
            const auto &history = m_inputHistory[param->id()];
            if(history.empty())
                continue;
            overrides.insert(param->id(), sampleValueAt(history, delayedTime));
        }

        applyToFixture(*context, fixture, overrides);
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

    // Re-derive the exposed ports' labels rather than keeping whatever was
    // saved. Parameter::readFromJson restores the stored name, so without this
    // a project saved before port naming improved would keep its old ambiguous
    // labels (every channel of a capability sharing one name) forever. Matching
    // on channelId means only the display name moves - connections are keyed on
    // the id and are untouched.
    if(m_state)
    {
        for(auto *capability : m_state->capabilities())
        {
            const int channelCount = capability->availableChannels().size();
            for(int i = 0; i < channelCount; ++i)
            {
                if(auto *param = findParameter(capability->channelId(i)))
                    param->setName(portNameFor(capability, i));
            }
        }
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
