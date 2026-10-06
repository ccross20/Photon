#include <QRandomGenerator>
#include "pixellistrandomizenode.h"
#include "graph/parameter/pixellistparameter.h"

namespace photon {

const QByteArray PixelListRandomizeNode::PixelsInParam = "pixelsIn";
const QByteArray PixelListRandomizeNode::SeedParam = "seed";
const QByteArray PixelListRandomizeNode::PixelsOutParam = "pixelsOut";

keira::NodeInformation PixelListRandomizeNode::info()
{
    keira::NodeInformation toReturn([](){return new PixelListRandomizeNode;});
    toReturn.name = "Randomize Pixels";
    toReturn.nodeId = "photon.pixels.randomize";
    toReturn.categories = {"Pixel"};
    return toReturn;
}

PixelListRandomizeNode::PixelListRandomizeNode() : keira::Node("photon.pixels.randomize")
{
    setName("Randomize Pixels");
}

void PixelListRandomizeNode::createParameters()
{
    m_inParam = new PixelListParameter(PixelsInParam, "Pixels In", {});
    m_seedParam = new keira::IntegerParameter(SeedParam, "Seed", 0);
    m_outParam = new PixelListParameter(PixelsOutParam, "Pixels Out", {}, keira::AllowMultipleOutput);

    addParameter(m_inParam);
    addParameter(m_seedParam);
    addParameter(m_outParam);
}

void PixelListRandomizeNode::evaluate(keira::EvaluationContext *) const
{
    auto pixels = m_inParam->value().value<QVector<PixelParameterData>>();

    // Fisher-Yates with Qt's generator rather than std::shuffle, whose
    // algorithm differs between standard libraries - a seed should give the
    // same order on every machine.
    QRandomGenerator generator(static_cast<quint32>(m_seedParam->value().toInt()));
    for(int i = int(pixels.size()) - 1; i > 0; --i)
        pixels.swapItemsAt(i, int(generator.bounded(i + 1)));

    m_outParam->setValue(QVariant::fromValue(pixels));
}

} // namespace photon
