#include "noisenode.h"

namespace photon {

keira::NodeInformation NoiseNode::info()
{
    keira::NodeInformation toReturn([](){return new NoiseNode;});
    toReturn.name = "Noise";
    toReturn.nodeId = "photon.math.noise";
    toReturn.categories = {"Math"};

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
    inputXParam = new keira::DecimalParameter("inputX","Input X", 2.0);
    addParameter(inputXParam);

    inputYParam = new keira::DecimalParameter("inputY","Input Y", 2.0);
    addParameter(inputYParam);


    scaleParam = new Point2DParameter("scale","Scale", QPointF(10.0,10.0));
    addParameter(scaleParam);

    inputFrequencyParam = new Point2DParameter("freq","Frequency", QPointF(10.0,10.0));
    addParameter(inputFrequencyParam);

    seedParam = new keira::IntegerParameter("seed","Seed", 0.0);
    addParameter(seedParam);

    noiseModeParam = new keira::OptionParameter("mode","Mode",{"Value","Value Fractal","Perlin","Perlin Fractal","Simplex","Simplex Fractal","Cellular","White Noise","Cubic","Cubic Fractal"},0);
    addParameter(noiseModeParam);

    minParam = new keira::DecimalParameter("min","Minimum", 0.0);
    addParameter(minParam);

    maxParam = new keira::DecimalParameter("max","Maximum", 1.0);
    addParameter(maxParam);

    outputParam = new keira::DecimalParameter("output","Output", 0.0, keira::AllowMultipleOutput);
    addParameter(outputParam);
}

void NoiseNode::evaluate(keira::EvaluationContext *t_context) const
{
    m_noise->setSeed(seedParam->value().toInt());

    double d = inputYParam->value().toDouble();
    qlonglong i = qlonglong(d)%100;
    double off = i + (d - floor(d));


    m_noise->setOffset(QPointF{0,off});
    m_noise->setMin(minParam->value().toInt());
    m_noise->setMax(maxParam->value().toInt());
    m_noise->setScale(scaleParam->value().value<QPointF>());
    m_noise->setFrequency(inputFrequencyParam->value().value<QPointF>());

    m_noise->setNoiseType(static_cast<NoiseGenerator::NoiseType>(noiseModeParam->value().toInt()));

    outputParam->setValue(m_noise->noise1D(inputXParam->value().toDouble(),0,1.0));
}

} // namespace photon
