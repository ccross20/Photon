#include "canvasoutputnode.h"

namespace photon {

const QByteArray CanvasOutputNode::TextureInput = "canvasInput";

keira::NodeInformation CanvasOutputNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasOutputNode;});
    toReturn.name = "Canvas Output";
    toReturn.nodeId = "photon.canvas.output";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasOutputNode::CanvasOutputNode() : keira::Node("photon.canvas.output")
{
    setName("Output");
}

void CanvasOutputNode::createParameters()
{
    m_textureInput = new RhiTextureParameter(TextureInput, "Canvas", RhiTextureData{},
                                             keira::AllowSingleInput);
    addParameter(m_textureInput);
}

void CanvasOutputNode::evaluate(keira::EvaluationContext *) const
{
    // The input param holds the upstream producer's handle at eval time; stash it
    // for the container to composite into the sink.
    m_lastTexture = m_textureInput->value().value<RhiTextureData>();
}

} // namespace photon
