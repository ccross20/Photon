#include <algorithm>
#include <QEasingCurve>
#include "easeddelaynode.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "routine/routineevaluationcontext.h"
#include "util/utils.h"

namespace photon {

class EasedDelayNode::Impl
{
public:
    keira::BooleanParameter *inputParam;
    keira::DecimalParameter *durationParam;
    keira::OptionParameter *easeParam;
    keira::DecimalParameter *valueParam;
    keira::BooleanParameter *delayedParam;

    // Evaluation state: written only from evaluate().
    bool initialized = false;
    double lastTime = 0.0;
    double progress = 0.0;   // 0 = false, 1 = true, linear
    bool delayed = false;
    QEasingCurve curve;
};

keira::NodeInformation EasedDelayNode::info()
{
    keira::NodeInformation toReturn([](){return new EasedDelayNode;});
    toReturn.name = "Eased Delay";
    toReturn.nodeId = "photon.math.eased-delay";
    toReturn.categories = {"Math"};
    return toReturn;
}

EasedDelayNode::EasedDelayNode() : keira::Node("photon.math.eased-delay"), m_impl(new Impl)
{
    setName("Eased Delay");
}

EasedDelayNode::~EasedDelayNode()
{
    delete m_impl;
}

void EasedDelayNode::createParameters()
{
    m_impl->inputParam = new keira::BooleanParameter("input", "Input", false);
    addParameter(m_impl->inputParam);

    m_impl->durationParam = new keira::DecimalParameter("duration", "Duration", 1.0);
    m_impl->durationParam->setMinimum(0.0);
    m_impl->durationParam->setSoftRange(0.0, 10.0);
    addParameter(m_impl->durationParam);

    // List index == QEasingCurve::Type value, as in the channel effects.
    m_impl->easeParam = new keira::OptionParameter("ease", "Ease", easeStrings(), QEasingCurve::InOutSine);
    addParameter(m_impl->easeParam);

    m_impl->valueParam = new keira::DecimalParameter("value", "Value", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);

    m_impl->delayedParam = new keira::BooleanParameter("delayed", "Delayed", false, keira::AllowMultipleOutput);
    addParameter(m_impl->delayedParam);
}

void EasedDelayNode::evaluate(keira::EvaluationContext *t_context) const
{
    // Song time inside a sequence, the running time in a bus - the same
    // clock the Delay node uses.
    auto *routineContext = dynamic_cast<RoutineEvaluationContext *>(t_context);
    const double now = routineContext ? routineContext->globalTime : t_context->time.elapsed;

    const bool input = m_impl->inputParam->value().toBool();
    const double target = input ? 1.0 : 0.0;

    if(!m_impl->initialized)
    {
        // Start settled on the current input rather than easing in from false.
        m_impl->initialized = true;
        m_impl->progress = target;
        m_impl->delayed = input;
    }
    else
    {
        // A backward jump (scrub, loop) moves nothing this frame.
        const double dt = std::max(0.0, now - m_impl->lastTime);
        const double duration = m_impl->durationParam->value().toDouble();
        if(duration <= 0.0)
        {
            m_impl->progress = target;
        }
        else
        {
            const double step = dt / duration;
            m_impl->progress = m_impl->progress < target
                                   ? std::min(target, m_impl->progress + step)
                                   : std::max(target, m_impl->progress - step);
        }
        if(m_impl->progress == target)
            m_impl->delayed = input;
    }
    m_impl->lastTime = now;

    m_impl->curve.setType(static_cast<QEasingCurve::Type>(m_impl->easeParam->value().toInt()));
    m_impl->valueParam->setValue(m_impl->curve.valueForProgress(m_impl->progress));
    m_impl->delayedParam->setValue(m_impl->delayed);
}

} // namespace photon
