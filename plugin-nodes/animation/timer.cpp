#include "timer.h"
#include "routine/routineevaluationcontext.h"

namespace photon {


keira::NodeInformation Timer::info()
{
    keira::NodeInformation toReturn([](){return new Timer;});
    toReturn.name = "Timer";
    toReturn.nodeId = "photon.animation.timer";
    toReturn.categories = {"Animation"};

    return toReturn;
}

Timer::Timer(): keira::Node("photon.animation.timer") {}

void Timer::createParameters()
{
    m_delayParam = new keira::DecimalParameter("delay", "Delay", 1);
    m_delayParam->setMinimum(0);
    addParameter(m_delayParam);

    m_loopParam = new keira::IntegerParameter("loop","Loop", 0);
    m_loopParam->setMinimum(0);
    addParameter(m_loopParam);

    m_resetParam = new keira::ButtonParameter("reset","Reset");
    addParameter(m_resetParam);

    m_countParam = new keira::IntegerParameter("count","Count", 0, keira::AllowMultipleOutput);
    m_countParam->setMinimum(0);
    addParameter(m_countParam);


}

void Timer::evaluate(keira::EvaluationContext *t_context) const
{
    double delay = m_delayParam->value().toDouble();
    int loop = m_loopParam->value().toInt();

    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    qlonglong count = std::floor(context->relativeTime / delay);


    if(loop > 0)
        count = count % loop;


    //qDebug() << delay << count;

    m_countParam->setValue(count);
}


} // namespace photon
