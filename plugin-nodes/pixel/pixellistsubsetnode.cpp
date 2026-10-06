#include "pixellistsubsetnode.h"
#include "graph/parameter/pixellistparameter.h"

namespace photon {

const QByteArray PixelListSubsetNode::PixelsInParam = "pixelsIn";
const QByteArray PixelListSubsetNode::OffsetParam = "offset";
const QByteArray PixelListSubsetNode::SizeParam = "size";
const QByteArray PixelListSubsetNode::PixelsOutParam = "pixelsOut";

keira::NodeInformation PixelListSubsetNode::info()
{
    keira::NodeInformation toReturn([](){return new PixelListSubsetNode;});
    toReturn.name = "Pixel Subset";
    toReturn.nodeId = "photon.pixels.subset";
    toReturn.categories = {"Pixel"};
    return toReturn;
}

PixelListSubsetNode::PixelListSubsetNode() : keira::Node("photon.pixels.subset")
{
    setName("Pixel Subset");
}

void PixelListSubsetNode::createParameters()
{
    m_inParam = new PixelListParameter(PixelsInParam, "Pixels In", {});

    m_offsetParam = new keira::IntegerParameter(OffsetParam, "Offset", 0);
    m_offsetParam->setMinimum(-100000);
    m_offsetParam->setMaximum(100000);

    m_sizeParam = new keira::IntegerParameter(SizeParam, "Size", 1);
    m_sizeParam->setMinimum(0);
    m_sizeParam->setMaximum(100000);

    m_outParam = new PixelListParameter(PixelsOutParam, "Pixels Out", {}, keira::AllowMultipleOutput);

    addParameter(m_inParam);
    addParameter(m_offsetParam);
    addParameter(m_sizeParam);
    addParameter(m_outParam);
}

void PixelListSubsetNode::evaluate(keira::EvaluationContext *) const
{
    const auto pixels = m_inParam->value().value<QVector<PixelParameterData>>();
    const int count = int(pixels.size());
    const int size = m_sizeParam->value().toInt();

    QVector<PixelParameterData> results;
    if(count > 0 && size > 0)
    {
        const int start = ((m_offsetParam->value().toInt() % count) + count) % count;
        const int length = ((size - 1) % count) + 1;   // 1..count, looping
        results.reserve(length);
        for(int i = 0; i < length; ++i)
            results.append(pixels[(start + i) % count]);
    }

    m_outParam->setValue(QVariant::fromValue(results));
}

} // namespace photon
