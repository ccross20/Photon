#include <algorithm>
#include "beatreducernode.h"

namespace photon {

keira::NodeInformation BeatReducerNode::info()
{
    keira::NodeInformation toReturn([](){return new BeatReducerNode;});
    toReturn.name = "Beat Reducer";
    toReturn.nodeId = "photon.animation.beat-reducer";
    toReturn.categories = {"Animation"};

    return toReturn;
}

BeatReducerNode::BeatReducerNode() : keira::Node("photon.animation.beat-reducer")
{
    setName("Beat Reducer");
}

void BeatReducerNode::createParameters()
{
    m_beatInParam = new keira::IntegerParameter("beatInput", "Beat", 0);
    addParameter(m_beatInParam);

    m_skipParam = new keira::IntegerParameter("skip", "Skip", 1);
    m_skipParam->setMinimum(0);
    addParameter(m_skipParam);

    m_offsetParam = new keira::IntegerParameter("offset", "Offset", 0);
    m_offsetParam->setMinimum(0);
    addParameter(m_offsetParam);

    m_beatOutParam = new keira::IntegerParameter("beatOutput", "Beat", 0, keira::AllowMultipleOutput);
    addParameter(m_beatOutParam);
}

void BeatReducerNode::evaluate(keira::EvaluationContext *) const
{
    const int beat = m_beatInParam->value().toInt();
    const int step = std::max(1, m_skipParam->value().toInt() + 1);
    const int offset = m_offsetParam->value().toInt();

    // Non-negative modulo, so the phase stays correct when (beat - offset) < 0.
    int phase = (beat - offset) % step;
    if(phase < 0)
        phase += step;

    // The most recent beat at or before `beat` that lands on the reduced grid.
    // Constant between hits, so a downstream trigger fires once per `step` beats.
    m_beatOutParam->setValue(beat - phase);
}

} // namespace photon
