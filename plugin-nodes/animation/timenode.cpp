#include "timenode.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

keira::NodeInformation TimeNode::info()
{
    keira::NodeInformation toReturn([](){return new TimeNode;});
    toReturn.name = "Time";
    toReturn.nodeId = "photon.animation.time";
    toReturn.categories = {"Animation"};

    return toReturn;
}

TimeNode::TimeNode() : keira::Node("photon.animation.time")
{
    setName("Time");
    // No inputs - it must re-run every frame to track the clock.
    setIsAlwaysDirty(true);
}

void TimeNode::createParameters()
{
    m_timeParam = new keira::DecimalParameter("time", "Time", 0.0, keira::AllowMultipleOutput);
    addParameter(m_timeParam);
}

void TimeNode::evaluate(keira::EvaluationContext *t_context) const
{
    // A graph opened directly in the node editor is ticked with a plain
    // keira::EvaluationContext; only a routine context carries the clock.
    auto *context = dynamic_cast<RoutineEvaluationContext *>(t_context);
    if(!context)
        return;

    m_timeParam->setValue(context->globalTime);
}

} // namespace photon
