#include "pulsenode.h"
#include "util/utils.h"

namespace photon {

class PulseNode::Impl
{
public:
    QEasingCurve m_easingIn;
    QEasingCurve m_easingOut;
};

keira::NodeInformation PulseNode::info()
{
    keira::NodeInformation toReturn([](){return new PulseNode;});
    toReturn.name = "Pulse";
    toReturn.nodeId = "photon.animation.pulse";
    toReturn.categories = {"Animation"};

    return toReturn;
}

PulseNode::PulseNode(): keira::Node("photon.animation.pulse"),m_impl(new Impl()) {}

PulseNode::~PulseNode()
{
    delete m_impl;
}


void PulseNode::createParameters()
{
    m_timeInputParam = new keira::DecimalParameter("time", "Time", 0);
    addParameter(m_timeInputParam);

    m_easeInTypeParam = new keira::OptionParameter("easeInType","Ease In Type",easeStrings(),0);
    addParameter(m_easeInTypeParam);
    m_easeInDurationParam = new keira::DecimalParameter("inDuration", "Ease In Duration", .2);
    m_easeInDurationParam->setMinimum(0.0);
    addParameter(m_easeInDurationParam);


    m_easeOutTypeParam = new keira::OptionParameter("easeOutType","Ease Out Type",easeStrings(),0);
    addParameter(m_easeOutTypeParam);
    m_easeOutDurationParam = new keira::DecimalParameter("outDuration", "Ease Out Duration", .2);
    m_easeOutDurationParam->setMinimum(0.0);
    addParameter(m_easeOutDurationParam);

    m_loopCountParam = new keira::IntegerParameter("loops","Loops", 1);
    m_loopCountParam->setMinimum(0);
    addParameter(m_loopCountParam);

    m_amplitudeParam = new keira::DecimalParameter("amp", "Amplitude", 1.0);
    addParameter(m_amplitudeParam);

    m_durationParam = new keira::DecimalParameter("duration", "Duration", 0.0);
    addParameter(m_durationParam);

    m_gapParam = new keira::DecimalParameter("gap", "gap", 1.0);
    addParameter(m_gapParam);

    m_outputParam = new keira::DecimalParameter("output", "Output", 0, keira::AllowMultipleOutput);
    addParameter(m_outputParam);


}

void PulseNode::evaluate(keira::EvaluationContext *t_context) const
{
    m_impl->m_easingIn.setType(static_cast<QEasingCurve::Type>(m_easeInTypeParam->value().toInt()));
    m_impl->m_easingOut.setType(static_cast<QEasingCurve::Type>(m_easeOutTypeParam->value().toInt()));

    double inDuration = m_easeInDurationParam->value().toDouble();
    double outDuration = m_easeOutDurationParam->value().toDouble();

    int totalLoopCount = m_loopCountParam->value().toInt();
    double time = m_timeInputParam->value().toDouble();
    double amplitude = m_amplitudeParam->value().toDouble();
    double duration = m_durationParam->value().toDouble();
    double gap = m_gapParam->value().toDouble();


    double totalDuration = inDuration  + duration+ outDuration;
    double loopTime = totalDuration + gap;
    double loopCount = std::floor(time / loopTime);
    double adjustedTime =  time - (loopCount * loopTime);
    double output = 0;

    if(loopCount > totalLoopCount)
    {
        m_outputParam->setValue(0);
        return;
    }

    if(adjustedTime < inDuration)
    {
        output = m_impl->m_easingIn.valueForProgress(adjustedTime/inDuration) * (amplitude);

    }
    else if(adjustedTime < inDuration + duration)
    {
        output = amplitude;
    }
    else if(adjustedTime < (outDuration + inDuration + duration))
    {
        adjustedTime -= inDuration + duration;
        output = (m_impl->m_easingOut.valueForProgress(1.0 - (adjustedTime / outDuration)) * (amplitude));

    }

    m_outputParam->setValue(output);
}

} // namespace photon
