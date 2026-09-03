#include <QRandomGenerator>
#include "randomcolornode.h"

namespace photon {

keira::NodeInformation RandomColorNode::info()
{
    keira::NodeInformation toReturn([](){return new RandomColorNode;});
    toReturn.name = "Random Color";
    toReturn.nodeId = "photon.color.random-color";
    toReturn.categories = {"Color"};

    return toReturn;
}

RandomColorNode::RandomColorNode() : keira::Node("photon.color.random-color")
{
    setName("Random Color");
}

void RandomColorNode::createParameters()
{
    m_seedParam = new keira::IntegerParameter("seed", "Seed", 1);
    m_seedParam->setMinimum(0);
    addParameter(m_seedParam);

    m_outputParam = new ColorParameter("output", "Output", QColor(Qt::red), keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void RandomColorNode::evaluate(keira::EvaluationContext *) const
{
    // Seed a fresh generator each time so the output is a pure function of the
    // seed - the same seed always yields the same hue.
    QRandomGenerator random(static_cast<quint32>(m_seedParam->value().toInt()));
    const double hue = random.generateDouble();   // [0, 1)

    m_outputParam->setValue(QColor::fromHsvF(hue, 1.0f, 1.0f));
}

} // namespace photon
