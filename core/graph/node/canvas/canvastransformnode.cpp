#include <cmath>
#include "canvastransformnode.h"

namespace photon {

const QByteArray CanvasTransformNode::Input = "input";
const QByteArray CanvasTransformNode::TranslateX = "translateX";
const QByteArray CanvasTransformNode::TranslateY = "translateY";
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

    m_translateX = new keira::DecimalParameter(TranslateX, "Translate X", 0.0);
    addParameter(m_translateX);

    m_translateY = new keira::DecimalParameter(TranslateY, "Translate Y", 0.0);
    addParameter(m_translateY);

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
    f[0] = float(m_translateX->value().toDouble());
    f[1] = float(m_translateY->value().toDouble());
    f[2] = float(m_rotation->value().toDouble() * M_PI / 180.0);   // degrees -> radians
    f[3] = float(m_scale->value().toDouble());
}

} // namespace photon
