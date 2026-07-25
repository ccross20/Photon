#ifndef PHOTON_BASECANVASNODE_H
#define PHOTON_BASECANVASNODE_H

#include <QByteArray>
#include <QSize>
#include "model/node.h"
#include "photon-global.h"
#include "graph/parameter/rhitextureparameter.h"

class QRhi;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;
class QRhiBuffer;
class QRhiSampler;

namespace photon {

// Common machinery for canvas producer/effect nodes: a fullscreen-triangle pass
// rendering into an owned RGBA8 output texture, using a shared vertex shader plus
// a subclass fragment shader, a std140 uniform block, and an optional single
// input texture. evaluate() runs on the main thread (inside the canvas subgraph's
// render). The node owns its output texture and reuses it across frames, releasing
// it (deferred to the main thread) on resize/destroy. See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT BaseCanvasNode : public keira::Node
{
public:
    BaseCanvasNode(const QByteArray &id);
    ~BaseCanvasNode();

    void evaluate(keira::EvaluationContext *) const override;

protected:
    // ":/canvas/shaders/<name>.frag.qsb" is loaded for the fragment stage.
    virtual QByteArray fragmentShaderName() const = 0;
    // Size (bytes) of the std140 uniform block. Must be > 0 and a multiple of 16.
    virtual quint32 uniformSize() const = 0;
    // Fill the (pre-sized, zeroed) uniform block for this frame.
    virtual void writeUniforms(QByteArray &out, const QSize &size) const = 0;
    // Effects override to true and return their input texture each frame.
    virtual bool samplesInput() const { return false; }
    virtual RhiTextureData currentInput() const { return {}; }

    // The output parameter the subclass creates in createParameters(); base sets it.
    RhiTextureParameter *m_outputParam = nullptr;

private:
    bool ensureResources(QRhi *rhi, const QSize &size, QRhiTexture *inputTex) const;
    void releaseAll() const;

    mutable QRhiTexture *m_output = nullptr;
    mutable QRhiTextureRenderTarget *m_rt = nullptr;
    mutable QRhiRenderPassDescriptor *m_rp = nullptr;
    mutable QRhiGraphicsPipeline *m_pipeline = nullptr;
    mutable QRhiShaderResourceBindings *m_srb = nullptr;
    mutable QRhiBuffer *m_ubuf = nullptr;
    mutable QRhiSampler *m_sampler = nullptr;
    mutable QSize m_size;
    mutable QRhiTexture *m_lastInput = nullptr;   // input handle the current SRB binds
};

} // namespace photon

#endif // PHOTON_BASECANVASNODE_H
