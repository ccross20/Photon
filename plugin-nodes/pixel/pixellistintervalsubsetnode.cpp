#include <algorithm>
#include "pixellistintervalsubsetnode.h"
#include "graph/parameter/pixellistparameter.h"

namespace photon {

const QByteArray PixelListIntervalSubsetNode::PixelsInParam = "pixelsIn";
const QByteArray PixelListIntervalSubsetNode::OffsetParam = "offset";
const QByteArray PixelListIntervalSubsetNode::SelectCountParam = "selectCount";
const QByteArray PixelListIntervalSubsetNode::SkipCountParam = "skipCount";
const QByteArray PixelListIntervalSubsetNode::LoopParam = "loop";
const QByteArray PixelListIntervalSubsetNode::PixelsOutParam = "pixelsOut";

keira::NodeInformation PixelListIntervalSubsetNode::info()
{
    keira::NodeInformation toReturn([](){return new PixelListIntervalSubsetNode;});
    toReturn.name = "Pixel Interval Subset";
    toReturn.nodeId = "photon.pixels.interval-subset";
    toReturn.categories = {"Pixel"};
    return toReturn;
}

PixelListIntervalSubsetNode::PixelListIntervalSubsetNode() : keira::Node("photon.pixels.interval-subset")
{
    setName("Pixel Interval Subset");
}

void PixelListIntervalSubsetNode::createParameters()
{
    m_inParam = new PixelListParameter(PixelsInParam, "Pixels In", {});

    m_offsetParam = new keira::IntegerParameter(OffsetParam, "Offset", 0);
    m_offsetParam->setMinimum(-100000);
    m_offsetParam->setMaximum(100000);

    m_selectParam = new keira::IntegerParameter(SelectCountParam, "Select Count", 1);
    m_selectParam->setMinimum(1);
    m_selectParam->setMaximum(100000);

    m_skipParam = new keira::IntegerParameter(SkipCountParam, "Skip Count", 1);
    m_skipParam->setMinimum(0);
    m_skipParam->setMaximum(100000);

    m_loopParam = new keira::BooleanParameter(LoopParam, "Loop", false);

    m_outParam = new PixelListParameter(PixelsOutParam, "Pixels Out", {}, keira::AllowMultipleOutput);

    addParameter(m_inParam);
    addParameter(m_offsetParam);
    addParameter(m_selectParam);
    addParameter(m_skipParam);
    addParameter(m_loopParam);
    addParameter(m_outParam);
}

void PixelListIntervalSubsetNode::evaluate(keira::EvaluationContext *) const
{
    const auto pixels = m_inParam->value().value<QVector<PixelParameterData>>();
    const int count = int(pixels.size());
    const int selectCount = std::max(m_selectParam->value().toInt(), 1);
    const int skipCount = std::max(m_skipParam->value().toInt(), 0);
    const int period = selectCount + skipCount;

    QVector<PixelParameterData> results;
    if(count > 0)
    {
        const int start = ((m_offsetParam->value().toInt() % count) + count) % count;
        // Looping walks the whole list once, wrapping at the end and stopping
        // just before the offset pixel comes round again.
        const int steps = m_loopParam->value().toBool() ? count : count - start;
        for(int step = 0; step < steps; ++step)
        {
            if(step % period < selectCount)
                results.append(pixels[(start + step) % count]);
        }
    }

    m_outParam->setValue(QVariant::fromValue(results));
}

} // namespace photon
