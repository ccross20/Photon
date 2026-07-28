#include <QColor>
#include "canvasgradientmapnode.h"
#include "graph/parameter/gradientparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray CanvasGradientMapNode::Source = "source";
const QByteArray CanvasGradientMapNode::GradientId = "gradient";
const QByteArray CanvasGradientMapNode::Channel = "channel";
const QByteArray CanvasGradientMapNode::Invert = "invert";
const QByteArray CanvasGradientMapNode::Mix = "mix";
const QByteArray CanvasGradientMapNode::Output = "output";

keira::NodeInformation CanvasGradientMapNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasGradientMapNode;});
    toReturn.name = "Gradient Map";
    toReturn.nodeId = "photon.canvas.gradient-map";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasGradientMapNode::CanvasGradientMapNode() : BaseCanvasNode("photon.canvas.gradient-map")
{
    setName("Gradient Map");
}

void CanvasGradientMapNode::createParameters()
{
    m_source = new RhiTextureParameter(Source, "Source", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_source);

    Gradient defaultGradient({ {0.0, QColor(0, 0, 0, 255)}, {1.0, QColor(255, 255, 255, 255)} });
    m_gradient = new GradientParameter(GradientId, "Gradient", defaultGradient);
    addParameter(m_gradient);

    m_channel = new keira::OptionParameter(Channel, "Channel",
        {"Luminance", "Red", "Green", "Blue", "Alpha"}, 0);
    addParameter(m_channel);

    m_invert = new keira::BooleanParameter(Invert, "Invert", false);
    addParameter(m_invert);

    m_mix = new keira::DecimalParameter(Mix, "Mix", 1.0);
    m_mix->setMinimum(0.0);
    m_mix->setMaximum(1.0);
    addParameter(m_mix);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasGradientMapNode::inputs() const
{
    return { m_source->value().value<RhiTextureData>() };
}

void CanvasGradientMapNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());

    const Gradient grad = m_gradient->value().value<Gradient>();
    const int count = qMin(grad.count(), MaxStops);

    // params
    f[0] = float(m_channel->value().toInt());
    f[1] = m_invert->value().toBool() ? 1.0f : 0.0f;
    f[2] = float(m_mix->value().toDouble());
    f[3] = float(count);

    // positions: 16 floats laid out linearly (positions[4]), starting at f[4]
    for (int i = 0; i < count; ++i)
        f[4 + i] = float(grad.stopAt(i).position);

    // colours: one vec4 per stop, starting at f[20] (byte 80)
    for (int i = 0; i < count; ++i) {
        const QColor c = grad.stopAt(i).color;
        f[20 + i * 4 + 0] = float(c.redF());
        f[20 + i * 4 + 1] = float(c.greenF());
        f[20 + i * 4 + 2] = float(c.blueF());
        f[20 + i * 4 + 3] = float(c.alphaF());
    }
}

} // namespace photon
