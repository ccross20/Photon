#include <cmath>
#include "canvascompositenode.h"
#include "graph/parameter/point2dparameter.h"

namespace photon {

const QByteArray CanvasCompositeNode::Base = "base";
const QByteArray CanvasCompositeNode::Top = "top";
const QByteArray CanvasCompositeNode::BlendMode = "blendMode";
const QByteArray CanvasCompositeNode::Alpha = "alpha";
const QByteArray CanvasCompositeNode::Position = "position";
const QByteArray CanvasCompositeNode::Scale = "scale";
const QByteArray CanvasCompositeNode::Rotation = "rotation";
const QByteArray CanvasCompositeNode::Origin = "origin";
const QByteArray CanvasCompositeNode::Output = "output";

keira::NodeInformation CanvasCompositeNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasCompositeNode;});
    toReturn.name = "Composite";
    toReturn.nodeId = "photon.canvas.composite";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasCompositeNode::CanvasCompositeNode() : BaseCanvasNode("photon.canvas.composite")
{
    setName("Composite");
}

void CanvasCompositeNode::createParameters()
{
    m_base = new RhiTextureParameter(Base, "Base", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_base);

    m_top = new RhiTextureParameter(Top, "Top", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_top);

    m_blendMode = new keira::OptionParameter(BlendMode, "Blend Mode",
        {"Normal", "Add", "Multiply", "Screen", "Overlay", "Subtract", "Difference", "Lighten", "Darken"}, 0);
    addParameter(m_blendMode);

    m_alpha = new keira::DecimalParameter(Alpha, "Alpha", 1.0);
    m_alpha->setMinimum(0.0);
    m_alpha->setMaximum(1.0);
    addParameter(m_alpha);

    m_position = new Point2DParameter(Position, "Position", QPointF(0.0, 0.0));
    addParameter(m_position);

    m_scale = new Point2DParameter(Scale, "Scale", QPointF(1.0, 1.0));
    addParameter(m_scale);

    m_rotation = new keira::DecimalParameter(Rotation, "Rotation", 0.0);
    addParameter(m_rotation);

    m_origin = new Point2DParameter(Origin, "Origin", QPointF(0.5, 0.5));
    addParameter(m_origin);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasCompositeNode::inputs() const
{
    // Base at binding 1, Top at binding 2. Only reached when both are present
    // (evaluate() handles the single-input passthrough cases).
    return { m_base->value().value<RhiTextureData>(),
             m_top->value().value<RhiTextureData>() };
}

void CanvasCompositeNode::evaluate(keira::EvaluationContext *t_context) const
{
    const RhiTextureData base = m_base->value().value<RhiTextureData>();
    const RhiTextureData top = m_top->value().value<RhiTextureData>();

    if (!base.texture && !top.texture) {
        m_outputParam->setValue(RhiTextureData{});
        return;
    }
    // With only one input, pass it straight through.
    if (!top.texture) {
        m_outputParam->setValue(base);
        return;
    }
    if (!base.texture) {
        m_outputParam->setValue(top);
        return;
    }

    BaseCanvasNode::evaluate(t_context);   // both present — render the composite
}

void CanvasCompositeNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    const QPointF pos = m_position->value().toPointF();
    const QPointF scale = m_scale->value().toPointF();
    const QPointF origin = m_origin->value().toPointF();

    f[0] = float(pos.x());    f[1] = float(pos.y());
    f[2] = float(scale.x());  f[3] = float(scale.y());
    f[4] = float(m_rotation->value().toDouble() * M_PI / 180.0);
    f[5] = float(origin.x()); f[6] = float(origin.y());
    f[7] = float(m_alpha->value().toDouble());
    f[8] = float(m_blendMode->value().toInt());
    f[9] = 0.0f; f[10] = 0.0f; f[11] = 0.0f;
}

} // namespace photon
