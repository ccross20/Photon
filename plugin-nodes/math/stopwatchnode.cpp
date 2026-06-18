#include "stopwatchnode.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

class StopwatchNode::Impl{
public:
    double lastValue;
    double triggerTime = 0;
    int count = 0;
};

keira::NodeInformation StopwatchNode::info()
{
    keira::NodeInformation toReturn([](){return new StopwatchNode;});
    toReturn.name = "Stopwatch";
    toReturn.nodeId = "photon.math.stopwatch";
    toReturn.categories = {"Animation"};

    return toReturn;
}

StopwatchNode::StopwatchNode(): keira::Node("photon.math.stopwatch"),m_impl(new Impl) {}

StopwatchNode::~StopwatchNode()
{
    delete m_impl;
}

void StopwatchNode::createParameters()
{
    m_modeParam = new keira::OptionParameter("mode","Trigger Mode",{"Value Change","Value Change > 0"},0);
    addParameter(m_modeParam);

    m_triggerInputParam = new keira::DecimalParameter("input", "Trigger", 1);
    addParameter(m_triggerInputParam);


    m_loopTimeInputParam = new keira::DecimalParameter("loop", "Loop Time", 0);
    m_loopTimeInputParam->setMinimum(0.0);
    addParameter(m_loopTimeInputParam);


    m_countParam = new keira::IntegerParameter("count","Trigger Count", 1, keira::AllowMultipleOutput);
    m_countParam->setMinimum(0);
    addParameter(m_countParam);

    m_timeOutputParam = new keira::DecimalParameter("output", "Output", 0, keira::AllowMultipleOutput);
    addParameter(m_timeOutputParam);

}

void StopwatchNode::evaluate(keira::EvaluationContext *t_context) const
{
    double trigger = m_triggerInputParam->value().toDouble();
    double loopTime = m_loopTimeInputParam->value().toDouble();
    int mode = m_modeParam->value().toInt();

    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    if(mode == 0)
    {
        if(m_impl->lastValue != trigger)
        {
            m_impl->lastValue = trigger;
            m_impl->triggerTime = context->globalTime;
            m_impl->count += 1;
        }
    }
    else {
        if(m_impl->lastValue != trigger)
        {
            m_impl->lastValue = trigger;
            if(trigger > 0.0)
            {
                m_impl->triggerTime = context->globalTime;
                m_impl->count += 1;
            }
        }
    }


    double elapsed = context->globalTime - m_impl->triggerTime;

    if(loopTime > 0.0)
    {
        elapsed -= (loopTime * std::floor(elapsed / loopTime));
    }

    m_countParam->setValue(m_impl->count);
    m_timeOutputParam->setValue(elapsed);
}

} // namespace photon
