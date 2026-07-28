#include "canvastilenode.h"
#include "graph/parameter/point2dparameter.h"

namespace photon {

const QByteArray CanvasTileNode::Input = "input";
const QByteArray CanvasTileNode::Tiles = "tiles";
const QByteArray CanvasTileNode::MirrorX = "mirrorX";
const QByteArray CanvasTileNode::MirrorY = "mirrorY";
const QByteArray CanvasTileNode::Output = "output";

keira::NodeInformation CanvasTileNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasTileNode;});
    toReturn.name = "Tile";
    toReturn.nodeId = "photon.canvas.tile";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasTileNode::CanvasTileNode() : BaseCanvasNode("photon.canvas.tile")
{
    setName("Tile");
}

void CanvasTileNode::createParameters()
{
    m_input = new RhiTextureParameter(Input, "Canvas", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_input);

    m_tiles = new Point2DParameter(Tiles, "Tiles", QPointF(2.0, 2.0));
    addParameter(m_tiles);

    m_mirrorX = new keira::BooleanParameter(MirrorX, "Mirror X", false);
    addParameter(m_mirrorX);

    m_mirrorY = new keira::BooleanParameter(MirrorY, "Mirror Y", false);
    addParameter(m_mirrorY);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasTileNode::inputs() const
{
    return { m_input->value().value<RhiTextureData>() };
}

void CanvasTileNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    const QPointF tiles = m_tiles->value().toPointF();
    f[0] = float(tiles.x());
    f[1] = float(tiles.y());
    f[2] = m_mirrorX->value().toBool() ? 1.0f : 0.0f;
    f[3] = m_mirrorY->value().toBool() ? 1.0f : 0.0f;
}

} // namespace photon
