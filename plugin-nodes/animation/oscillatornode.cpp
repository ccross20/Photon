#include <cmath>
#include "oscillatornode.h"

namespace photon {

keira::NodeInformation OscillatorNode::info()
{
    keira::NodeInformation toReturn([](){return new OscillatorNode;});
    toReturn.name = "Oscillator";
    toReturn.nodeId = "photon.animation.oscillator";
    toReturn.categories = {"Animation"};

    return toReturn;
}

OscillatorNode::OscillatorNode() : keira::Node("photon.animation.oscillator")
{
    setName("Oscillator");
}

void OscillatorNode::createParameters()
{
    m_timeParam = new keira::DecimalParameter("time", "Time", 0.0);
    addParameter(m_timeParam);

    // Seconds for one full cycle of the wave.
    m_speedParam = new keira::DecimalParameter("speed", "Speed", 1.0);
    addParameter(m_speedParam);

    m_minParam = new keira::DecimalParameter("min", "Min", 0.0);
    addParameter(m_minParam);
    m_maxParam = new keira::DecimalParameter("max", "Max", 1.0);
    addParameter(m_maxParam);

    m_outputParam = new keira::DecimalParameter("output", "Output", 0.0, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void OscillatorNode::evaluate(keira::EvaluationContext *) const
{
    const double time = m_timeParam->value().toDouble();
    const double speed = m_speedParam->value().toDouble();
    const double min = m_minParam->value().toDouble();
    const double max = m_maxParam->value().toDouble();

    // A cosine wave starting at its low point: Min at Time 0, Max halfway
    // through the cycle (Time = Speed / 2), back to Min at Time = Speed - so a
    // fade driven by it starts dark.
    constexpr double kTwoPi = 6.28318530717958647692;
    double unit = 0.0;
    if(std::abs(speed) > 1e-9)
    {
        const double phase = (time / speed) * kTwoPi;
        unit = 0.5 - 0.5 * std::cos(phase);
    }

    m_outputParam->setValue(min + unit * (max - min));
}

} // namespace photon
