#include "delaynode.h"
#include "model/parameter/decimalparameter.h"
#include "routine/routineevaluationcontext.h"
#include <deque>

namespace photon {

const QByteArray DelayNode::Input    = "input";
const QByteArray DelayNode::Output   = "output";
const QByteArray DelayNode::MaxDelay = "maxDelay";

struct DelaySample { double time; double value; };

class DelayNode::Impl
{
public:
    keira::DecimalParameter *inputParam    = nullptr;
    keira::DecimalParameter *outputParam   = nullptr;
    keira::DecimalParameter *maxDelayParam = nullptr;
    std::deque<DelaySample>  buffer;
};

keira::NodeInformation DelayNode::info()
{
    keira::NodeInformation toReturn([]() { return new DelayNode; });
    toReturn.name       = "Delay";
    toReturn.nodeId     = "photon.math.delay";
    toReturn.categories = {"Math"};
    toReturn.graphs     = QByteArrayList{"fixture", "surface"};
    return toReturn;
}

DelayNode::DelayNode() : keira::Node("photon.math.delay"), m_impl(new Impl)
{
    setName("Delay");
    setIsAlwaysDirty(true);
}

DelayNode::~DelayNode()
{
    delete m_impl;
}

void DelayNode::createParameters()
{
    m_impl->inputParam = new keira::DecimalParameter(Input, "Input", 0.0);
    addParameter(m_impl->inputParam);

    m_impl->maxDelayParam = new keira::DecimalParameter(MaxDelay, "Max Delay", 10.0);
    m_impl->maxDelayParam->setMinimum(0.0);
    m_impl->maxDelayParam->setMaximum(60.0);
    addParameter(m_impl->maxDelayParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", 0.0,
                                                       keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void DelayNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto *ctx = static_cast<RoutineEvaluationContext *>(t_context);
    const double now        = ctx->globalTime;
    const double offset     = ctx->timeOffset;
    const double maxDelay   = m_impl->maxDelayParam->value().toDouble();
    const double inputValue = m_impl->inputParam->value().toDouble();

    // Record this frame's sample
    m_impl->buffer.push_back({now, inputValue});

    // Prune samples older than we'll ever need
    const double cutoff = now - maxDelay - 1.0;
    while (m_impl->buffer.size() > 1 && m_impl->buffer.front().time < cutoff)
        m_impl->buffer.pop_front();

    // No offset — pass the value through immediately
    if (qFuzzyIsNull(offset)) {
        m_impl->outputParam->setValue(inputValue);
        return;
    }

    const double targetTime = now - offset;

    // Not enough history yet — output the oldest sample we have
    if (m_impl->buffer.front().time >= targetTime) {
        m_impl->outputParam->setValue(m_impl->buffer.front().value);
        return;
    }

    // Binary search for the pair of samples that bracket targetTime
    int lo = 0, hi = static_cast<int>(m_impl->buffer.size()) - 1;
    while (lo + 1 < hi) {
        const int mid = (lo + hi) / 2;
        if (m_impl->buffer[mid].time <= targetTime)
            lo = mid;
        else
            hi = mid;
    }

    // Linear interpolation between the two bracketing samples
    const auto &a = m_impl->buffer[lo];
    const auto &b = m_impl->buffer[hi];
    const double span = b.time - a.time;
    if (span < 1e-9) {
        m_impl->outputParam->setValue(a.value);
        return;
    }
    const double t = (targetTime - a.time) / span;
    m_impl->outputParam->setValue(a.value + t * (b.value - a.value));
}

} // namespace photon
