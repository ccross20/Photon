#ifndef PHOTON_CANVASOUTPUTNODE_H
#define PHOTON_CANVASOUTPUTNODE_H

#include "model/node.h"
#include "photon-global.h"
#include "graph/parameter/rhitextureparameter.h"

namespace photon {

// Auto-created terminal "Output" node inside a canvas subgraph. Whatever texture
// is wired into it is the subgraph's result: after the inner graph evaluates, the
// CanvasSubGraphNode reads inputTexture() and composites it into the canvas sink.
class PHOTONCORE_EXPORT CanvasOutputNode : public keira::Node
{
public:
    const static QByteArray TextureInput;

    CanvasOutputNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

    // The texture connected to this node's input on the last evaluation (null if
    // nothing is wired in).
    RhiTextureData inputTexture() const { return m_lastTexture; }

private:
    RhiTextureParameter *m_textureInput;
    mutable RhiTextureData m_lastTexture;
};

} // namespace photon

#endif // PHOTON_CANVASOUTPUTNODE_H
