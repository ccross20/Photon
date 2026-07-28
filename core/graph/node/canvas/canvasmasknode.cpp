#include "canvasmasknode.h"

namespace photon {

const QByteArray CanvasMaskNode::Source = "source";
const QByteArray CanvasMaskNode::Mask = "mask";
const QByteArray CanvasMaskNode::Channel = "channel";
const QByteArray CanvasMaskNode::Invert = "invert";
const QByteArray CanvasMaskNode::Output = "output";

keira::NodeInformation CanvasMaskNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasMaskNode;});
    toReturn.name = "Mask";
    toReturn.nodeId = "photon.canvas.mask";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasMaskNode::CanvasMaskNode() : BaseCanvasNode("photon.canvas.mask")
{
    setName("Mask");
}

void CanvasMaskNode::createParameters()
{
    m_source = new RhiTextureParameter(Source, "Source", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_source);

    m_mask = new RhiTextureParameter(Mask, "Mask", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_mask);

    m_channel = new keira::OptionParameter(Channel, "Channel",
        {"Alpha", "Luminance", "Red", "Green", "Blue"}, 0);
    addParameter(m_channel);

    m_invert = new keira::BooleanParameter(Invert, "Invert", false);
    addParameter(m_invert);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasMaskNode::inputs() const
{
    // Source at binding 1, Mask at binding 2 (both present — evaluate() handles the
    // source-only passthrough).
    return { m_source->value().value<RhiTextureData>(),
             m_mask->value().value<RhiTextureData>() };
}

void CanvasMaskNode::evaluate(keira::EvaluationContext *t_context) const
{
    const RhiTextureData source = m_source->value().value<RhiTextureData>();
    const RhiTextureData mask = m_mask->value().value<RhiTextureData>();

    if (!source.texture) {
        m_outputParam->setValue(RhiTextureData{});
        return;
    }
    if (!mask.texture) {
        m_outputParam->setValue(source);   // no mask — pass the source through
        return;
    }

    BaseCanvasNode::evaluate(t_context);
}

void CanvasMaskNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    f[0] = float(m_channel->value().toInt());
    f[1] = m_invert->value().toBool() ? 1.0f : 0.0f;
    f[2] = 0.0f;
    f[3] = 0.0f;
}

} // namespace photon
