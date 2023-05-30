#include "noisenode.h"

namespace photon {

keira::NodeInformation NoiseNode::info()
{
    keira::NodeInformation toReturn([](){return new NoiseNode;});
    toReturn.name = "Noise";
    toReturn.nodeId = "photon.math.noise";

    return toReturn;
}

NoiseNode::NoiseNode() : keira::Node("photon.math.noise")
{
    setName("Noise");

    m_noise = new NoiseGenerator;
}

NoiseNode::~NoiseNode()
{
    delete m_noise;
}

void NoiseNode::createParameters()
{
    inputParam = new keira::DecimalParameter("input","Input", 2.0);
    addParameter(inputParam);


    scaleParam = new keira::DecimalParameter("scale","Scale", 10.0);
    addParameter(scaleParam);

    strengthParam = new keira::DecimalParameter("strength","Amount", 0.0);
    addParameter(strengthParam);

    seedParam = new keira::IntegerParameter("seed","Seed", 0.0);
    addParameter(seedParam);

    noiseModeParam = new keira::OptionParameter("mode","Mode",{"Value","Value Fractal","Perlin","Perlin Fractal","Simplex","Simplex Fractal","Cellular","White Noise","Cubic","Cubic Fractal"},0);
    addParameter(noiseModeParam);

    outputParam = new keira::DecimalParameter("output","Output", 0.0, keira::AllowMultipleOutput);
    addParameter(outputParam);
}

void NoiseNode::evaluate(keira::EvaluationContext *t_context) const
{
    m_noise->setSeed(seedParam->value().toInt());
    m_noise->setNoiseType(static_cast<NoiseGenerator::NoiseType>(noiseModeParam->value().toInt()));

    outputParam->setValue(m_noise->noise1D(inputParam->value().toDouble() * scaleParam->value().toDouble(),0,strengthParam->value().toDouble()));
}

} // namespace photon
