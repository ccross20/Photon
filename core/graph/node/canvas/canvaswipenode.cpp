#include <cmath>
#include "canvaswipenode.h"

namespace photon {

const QByteArray CanvasWipeNode::From = "from";
const QByteArray CanvasWipeNode::To = "to";
const QByteArray CanvasWipeNode::Progress = "progress";
const QByteArray CanvasWipeNode::Angle = "angle";
const QByteArray CanvasWipeNode::Softness = "softness";
const QByteArray CanvasWipeNode::Output = "output";

keira::NodeInformation CanvasWipeNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasWipeNode;});
    toReturn.name = "Wipe";
    toReturn.nodeId = "photon.canvas.wipe";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasWipeNode::CanvasWipeNode() : BaseCanvasNode("photon.canvas.wipe")
{
    setName("Wipe");
}

void CanvasWipeNode::createParameters()
{
    m_from = new RhiTextureParameter(From, "From", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_from);

    m_to = new RhiTextureParameter(To, "To", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_to);

    m_progress = new keira::DecimalParameter(Progress, "Progress", 0.0);
    m_progress->setMinimum(0.0);
    m_progress->setMaximum(1.0);
    addParameter(m_progress);

    m_angle = new keira::DecimalParameter(Angle, "Angle", 0.0);
    addParameter(m_angle);

    m_softness = new keira::DecimalParameter(Softness, "Softness", 0.0);
    m_softness->setMinimum(0.0);
    m_softness->setMaximum(1.0);
    addParameter(m_softness);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasWipeNode::inputs() const
{
    // From at binding 1, To at binding 2 (both present — evaluate() handles the
    // single-input passthrough cases).
    return { m_from->value().value<RhiTextureData>(),
             m_to->value().value<RhiTextureData>() };
}

void CanvasWipeNode::evaluate(keira::EvaluationContext *t_context) const
{
    const RhiTextureData from = m_from->value().value<RhiTextureData>();
    const RhiTextureData to = m_to->value().value<RhiTextureData>();

    if (!from.texture && !to.texture) {
        m_outputParam->setValue(RhiTextureData{});
        return;
    }
    if (!to.texture) {
        m_outputParam->setValue(from);
        return;
    }
    if (!from.texture) {
        m_outputParam->setValue(to);
        return;
    }

    BaseCanvasNode::evaluate(t_context);
}

void CanvasWipeNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    f[0] = float(m_progress->value().toDouble());
    f[1] = float(m_angle->value().toDouble() * M_PI / 180.0);
    f[2] = float(m_softness->value().toDouble());
    f[3] = 0.0f;
}

} // namespace photon
