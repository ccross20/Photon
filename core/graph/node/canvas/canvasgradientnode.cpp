#include <cmath>
#include <QColor>
#include "canvasgradientnode.h"
#include "graph/parameter/gradientparameter.h"
#include "graph/parameter/point2dparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray CanvasGradientNode::Type = "type";
const QByteArray CanvasGradientNode::GradientId = "gradient";
const QByteArray CanvasGradientNode::Angle = "angle";
const QByteArray CanvasGradientNode::Center = "center";
const QByteArray CanvasGradientNode::Radius = "radius";
const QByteArray CanvasGradientNode::Output = "output";

keira::NodeInformation CanvasGradientNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasGradientNode;});
    toReturn.name = "Gradient";
    toReturn.nodeId = "photon.canvas.gradient";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasGradientNode::CanvasGradientNode() : BaseCanvasNode("photon.canvas.gradient")
{
    setName("Gradient");
}

void CanvasGradientNode::createParameters()
{
    m_type = new keira::OptionParameter(Type, "Type", {"Linear", "Radial", "Angular"}, 0);
    addParameter(m_type);

    Gradient defaultGradient({ {0.0, QColor(0, 0, 0, 255)}, {1.0, QColor(255, 255, 255, 255)} });
    m_gradient = new GradientParameter(GradientId, "Gradient", defaultGradient);
    addParameter(m_gradient);

    m_angle = new keira::DecimalParameter(Angle, "Angle", 0.0);
    addParameter(m_angle);

    m_center = new Point2DParameter(Center, "Center", QPointF(0.5, 0.5));
    addParameter(m_center);

    m_radius = new keira::DecimalParameter(Radius, "Radius", 0.5);
    m_radius->setMinimum(0.0);
    addParameter(m_radius);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void CanvasGradientNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());

    const Gradient grad = m_gradient->value().value<Gradient>();
    const int count = qMin(grad.count(), MaxStops);

    // header
    f[0] = float(m_type->value().toInt());
    f[1] = float(m_angle->value().toDouble() * M_PI / 180.0);
    f[2] = float(count);
    f[3] = 0.0f;

    // shape (centre + radius)
    const QPointF center = m_center->value().value<QPointF>();
    f[4] = float(center.x());
    f[5] = float(center.y());
    f[6] = float(m_radius->value().toDouble());
    f[7] = 0.0f;

    // positions: 16 floats laid out linearly (positions[4] as vec4), starting at f[8]
    for (int i = 0; i < count; ++i)
        f[8 + i] = float(grad.stopAt(i).position);

    // colours: one vec4 per stop, starting at f[24] (byte 96)
    for (int i = 0; i < count; ++i) {
        const QColor c = grad.stopAt(i).color;
        f[24 + i * 4 + 0] = float(c.redF());
        f[24 + i * 4 + 1] = float(c.greenF());
        f[24 + i * 4 + 2] = float(c.blueF());
        f[24 + i * 4 + 3] = float(c.alphaF());
    }
}

} // namespace photon
