#include "retimenode.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

class RetimeNode::Impl
{
public:
    double lastInputTime = 0.0;
    double outputTime = 0.0;
    bool initialized = false;
};

keira::NodeInformation RetimeNode::info()
{
    keira::NodeInformation toReturn([](){return new RetimeNode;});
    toReturn.name = "Retime";
    toReturn.nodeId = "photon.animation.retime";
    toReturn.categories = {"Animation"};

    return toReturn;
}

RetimeNode::RetimeNode(): keira::Node("photon.animation.retime"),m_impl(new Impl)
{
    setName("Retime");
    // Must keep integrating every frame even with nothing wired into Time
    // (the fallback-to-global-time case) or Speed, since normal dirty
    // propagation from an upstream node isn't there to drive it - see
    // TimeNode, which needs the same for the same reason.
    setIsAlwaysDirty(true);
}

RetimeNode::~RetimeNode()
{
    delete m_impl;
}

void RetimeNode::createParameters()
{
    m_timeParam = new keira::DecimalParameter("time", "Time", 0.0);
    addParameter(m_timeParam);

    m_speedParam = new keira::DecimalParameter("speed", "Speed", 1.0);
    addParameter(m_speedParam);

    m_outputParam = new keira::DecimalParameter("output", "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void RetimeNode::evaluate(keira::EvaluationContext *t_context) const
{
    // Time falls back to the eval context's own global time when nothing is
    // wired in, so dropping this node down with no input still does
    // something sensible - the common case being "retime the global clock".
    // A graph opened directly in the node editor is ticked with a plain
    // keira::EvaluationContext rather than a routine one; only matters here
    // when there's no Time input to fall back on it for.
    double inputTime;
    if(m_timeParam->inputParameter())
    {
        inputTime = m_timeParam->value().toDouble();
    }
    else
    {
        auto *context = dynamic_cast<RoutineEvaluationContext *>(t_context);
        inputTime = context ? context->globalTime : 0.0;
    }

    const double speed = m_speedParam->value().toDouble();

    if(!m_impl->initialized)
    {
        // Snap to the input on first sight rather than integrating from 0 -
        // an input fed a large elapsed-time value would otherwise take one
        // huge first step, jumping the output far from "now".
        m_impl->lastInputTime = inputTime;
        m_impl->outputTime = inputTime;
        m_impl->initialized = true;
    }
    else
    {
        // Integrate from how far the input itself moved, not from any fixed
        // frame delta - so retiming stays correct however often (or
        // irregularly) this evaluates. Speed only ever scales *this* step,
        // so a speed change takes effect going forward without moving the
        // output value that already exists.
        m_impl->outputTime += (inputTime - m_impl->lastInputTime) * speed;
        m_impl->lastInputTime = inputTime;
    }

    m_outputParam->setValue(m_impl->outputTime);
}

} // namespace photon
