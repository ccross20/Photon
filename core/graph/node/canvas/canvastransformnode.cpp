#include <cmath>
#include "canvastransformnode.h"

namespace photon {

const QByteArray CanvasTransformNode::Input = "input";
const QByteArray CanvasTransformNode::Translate = "translate";
const QByteArray CanvasTransformNode::Rotation = "rotation";
const QByteArray CanvasTransformNode::Scale = "scale";
const QByteArray CanvasTransformNode::Output = "output";

keira::NodeInformation CanvasTransformNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasTransformNode;});
    toReturn.name = "Transform";
    toReturn.nodeId = "photon.canvas.transform";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasTransformNode::CanvasTransformNode() : BaseCanvasNode("photon.canvas.transform")
{
    setName("Transform");
}

void CanvasTransformNode::createParameters()
{
    m_input = new RhiTextureParameter(Input, "Canvas", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_input);

    m_translate = new Point2DParameter(Translate, "Translate", QPointF(0.0, 0.0));
    addParameter(m_translate);

    m_rotation = new keira::DecimalParameter(Rotation, "Rotation", 0.0);
    addParameter(m_rotation);

    m_scale = new keira::DecimalParameter(Scale, "Scale", 1.0);
    m_scale->setMinimum(0.001);
    addParameter(m_scale);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasTransformNode::inputs() const
{
    return { m_input->value().value<RhiTextureData>() };
}

void CanvasTransformNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    const QPointF translate = m_translate->value().value<QPointF>();
    f[0] = float(translate.x());
    f[1] = float(translate.y());
    f[2] = float(m_rotation->value().toDouble() * M_PI / 180.0);   // degrees -> radians
    f[3] = float(m_scale->value().toDouble());
}

} // namespace photon
