#include <algorithm>
#include "switchnode.h"
#include "model/graph.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/anyparameter.h"

namespace photon {

const QByteArray SwitchNode::Select = "select";
const QByteArray SwitchNode::Output = "output";
const QByteArray SwitchNode::InputPrefix = "input";

keira::NodeInformation SwitchNode::info()
{
    keira::NodeInformation toReturn([](){return new SwitchNode;});
    toReturn.name = "Switch";
    toReturn.nodeId = "photon.node.switch";
    toReturn.categories = {"Utility"};

    return toReturn;
}

SwitchNode::SwitchNode() : keira::Node("photon.node.switch")
{
    setName("Switch");
}

keira::AnyParameter *SwitchNode::makeInput(int t_index) const
{
    auto *param = new keira::AnyParameter(InputPrefix + QByteArray::number(t_index),
                                          QString("Input %1").arg(t_index + 1),
                                          keira::AllowSingleInput);
    // Once the switch's type is locked, every new input should be born
    // already conforming to it, not generic.
    if(m_outputParam && m_outputParam->isResolved())
        param->resolve(m_outputParam->typeId());
    return param;
}

void SwitchNode::createParameters()
{
    m_selectParam = new keira::IntegerParameter(Select, "Select", 0);
    m_selectParam->setMinimum(0);
    addParameter(m_selectParam);

    m_outputParam = new keira::AnyParameter(Output, "Output", keira::AllowMultipleOutput);
    addParameter(m_outputParam);

    auto *first = makeInput(0);
    addParameter(first);
    m_inputs.append(first);
}

// Re-derives our bookkeeping pointers from whatever parameters actually exist
// on the node. Needed after readFromJson, since the base class may have
// created extra "input" AnyParameters directly (for a switch that had more
// than one input wired up when it was saved) without going through us.
void SwitchNode::rebuildBookkeeping()
{
    m_selectParam = nullptr;
    m_outputParam = nullptr;
    m_inputs.clear();

    for(auto *param : parameters())
    {
        if(param->id() == Select)
            m_selectParam = static_cast<keira::IntegerParameter*>(param);
        else if(param->id() == Output)
            m_outputParam = static_cast<keira::AnyParameter*>(param);
        else if(param->id().startsWith(InputPrefix))
            m_inputs.append(static_cast<keira::AnyParameter*>(param));
    }

    std::sort(m_inputs.begin(), m_inputs.end(), [](keira::AnyParameter *a, keira::AnyParameter *b) {
        return a->id().mid(InputPrefix.length()).toInt() < b->id().mid(InputPrefix.length()).toInt();
    });
}

void SwitchNode::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    keira::Node::readFromJson(t_json, t_library);
    rebuildBookkeeping();
}

// If the last (always-empty-until-now) input just got wired, grow the list by
// one so there's always a fresh empty input waiting at the end.
void SwitchNode::growIfNeeded()
{
    if(!graph() || m_inputs.isEmpty() || !m_inputs.last()->hasInput())
        return;

    auto *next = makeInput(m_inputs.size());
    graph()->runCommand([this, next]() {
        addParameter(next);
        m_inputs.append(next);
        portsChanged();
    });
}

void SwitchNode::inputParameterConnected(keira::Parameter *t_source)
{
    // Find which of our own switch inputs just received this connection (not
    // the Select port, which takes its own upstream driver). Growth has to
    // happen regardless of whether either side is still generic — e.g. two
    // freshly-connected Switch nodes wired "any" to "any" still use up this
    // input slot and need a fresh empty one after it.
    for(auto *input : m_inputs)
    {
        if(!input->inputParameters().contains(t_source))
            continue;

        reconcileConnection(input, t_source);
        growIfNeeded();
        return;
    }
}

void SwitchNode::outputParameterConnected(keira::Parameter *t_target)
{
    // Preserve the base bookkeeping (dependent-node tracking for dirty
    // propagation) that this hook normally does.
    keira::Node::outputParameterConnected(t_target);

    reconcileConnection(m_outputParam, t_target);
}

// Resolves whichever of our own ports aren't already resolved to t_type. If
// that changed anything, spreads to directly-connected Switch neighbors
// (downstream via our output, upstream via any input) so a whole chain
// resolves together. If nothing changed here (we were already fully
// resolved), we don't re-walk our neighbors — they either already have the
// type too, or reconcileConnection is about to push it to the one that
// doesn't. That's what keeps this from looping forever around a mesh of
// mutually-connected, already-resolved switches.
void SwitchNode::applyType(const QByteArray &t_type)
{
    if(t_type.isEmpty() || t_type == keira::AnyParameter::ParameterId || !m_outputParam)
        return;

    bool changed = false;
    if(!m_outputParam->isResolved())
    {
        m_outputParam->resolve(t_type);
        changed = true;
    }
    for(auto *input : m_inputs)
    {
        if(!input->isResolved())
        {
            input->resolve(t_type);
            changed = true;
        }
    }

    if(!changed)
        return;

    // Ports only repaint when the view rebuilds them; a plain resolve() is
    // otherwise invisible until something else (e.g. reselecting the node)
    // happens to force a redraw.
    portsChanged();

    for(auto *downstream : m_outputParam->outputParameters())
    {
        if(auto *neighbor = dynamic_cast<SwitchNode*>(downstream->node()))
            neighbor->applyType(t_type);
    }
    for(auto *input : m_inputs)
    {
        if(!input->hasInput())
            continue;
        if(auto *neighbor = dynamic_cast<SwitchNode*>(input->inputParameter()->node()))
            neighbor->applyType(t_type);
    }
}

// A new wire just formed between our port (ours) and a foreign one (theirs).
// Whichever side already has a concrete type wins; apply it to both — this
// is what makes an already-typed switch push its type onto a switch it's
// only now being connected to, not just one that was still generic at
// connect time.
void SwitchNode::reconcileConnection(keira::Parameter *t_ours, keira::Parameter *t_theirs)
{
    QByteArray type = t_ours->typeId();
    if(type.isEmpty() || type == keira::AnyParameter::ParameterId)
        type = t_theirs->typeId();
    if(type.isEmpty() || type == keira::AnyParameter::ParameterId)
        return;

    applyType(type);
    if(auto *neighbor = dynamic_cast<SwitchNode*>(t_theirs->node()))
        neighbor->applyType(type);
}

void SwitchNode::evaluate(keira::EvaluationContext *) const
{
    if(!m_selectParam || !m_outputParam || m_inputs.isEmpty())
        return;

    const int index = qBound(0, m_selectParam->value().toInt(), m_inputs.size() - 1);
    m_outputParam->setValue(m_inputs[index]->value());
}

} // namespace photon
