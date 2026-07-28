#include "canvasnoisenode.h"
#include "graph/parameter/point2dparameter.h"

namespace photon {

const QByteArray CanvasNoiseNode::Mode = "mode";
const QByteArray CanvasNoiseNode::Offset = "offset";
const QByteArray CanvasNoiseNode::Scale = "scale";
const QByteArray CanvasNoiseNode::Complexity = "complexity";
const QByteArray CanvasNoiseNode::Warp = "warp";
const QByteArray CanvasNoiseNode::Seed = "seed";
const QByteArray CanvasNoiseNode::Time = "time";
const QByteArray CanvasNoiseNode::Output = "output";

keira::NodeInformation CanvasNoiseNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasNoiseNode;});
    toReturn.name = "Noise";
    toReturn.nodeId = "photon.canvas.noise";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasNoiseNode::CanvasNoiseNode() : BaseCanvasNode("photon.canvas.noise")
{
    setName("Noise");
}

void CanvasNoiseNode::createParameters()
{
    m_mode = new keira::OptionParameter(Mode, "Mode", {"Value", "Fractal", "Cellular", "Flow"}, 1);
    addParameter(m_mode);

    m_offset = new Point2DParameter(Offset, "Offset", QPointF(0.0, 0.0));
    addParameter(m_offset);

    m_scale = new keira::DecimalParameter(Scale, "Scale", 4.0);
    m_scale->setMinimum(0.001);
    addParameter(m_scale);

    m_complexity = new keira::IntegerParameter(Complexity, "Complexity", 4);
    m_complexity->setMinimum(1);
    m_complexity->setMaximum(8);
    addParameter(m_complexity);

    m_warp = new keira::DecimalParameter(Warp, "Warp", 0.0);
    m_warp->setMinimum(0.0);
    addParameter(m_warp);

    m_seed = new keira::IntegerParameter(Seed, "Seed", 0);
    addParameter(m_seed);

    m_time = new keira::DecimalParameter(Time, "Time", 0.0);
    addParameter(m_time);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void CanvasNoiseNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    const QPointF offset = m_offset->value().value<QPointF>();
    f[0] = float(offset.x());
    f[1] = float(offset.y());
    f[2] = float(m_scale->value().toDouble());
    f[3] = float(m_time->value().toDouble());
    f[4] = float(m_mode->value().toInt());
    f[5] = float(m_complexity->value().toInt());
    f[6] = float(m_warp->value().toDouble());   // domain-warp amount
    f[7] = 2.0f;                                 // lacunarity
    f[8] = float(m_seed->value().toInt());       // seed
}

} // namespace photon
