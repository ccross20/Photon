#ifndef PHOTON_CANVASDMXSAMPLER_H
#define PHOTON_CANVASDMXSAMPLER_H

#include <QVector>
#include <QPointF>
#include <QColor>
#include <QSize>
#include "photon-global.h"

class QRhi;
class QRhiCommandBuffer;
class QRhiResourceUpdateBatch;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;
class QRhiSampler;
class QRhiBuffer;

namespace photon {

// GPU gather for canvas DMX output (5b): instead of reading the whole canvas back
// each frame, this samples the canvas sink texture at just the pixel-sample UVs
// (one point drawn per sample into a packed N-texel target) and reads back only
// those N texels. All calls are main-thread only. See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasDmxSampler
{
public:
    CanvasDmxSampler();
    ~CanvasDmxSampler();

    // Records the gather pass into the current offscreen frame: samples `sink` at
    // each uv and queues a readback. `preBatch` (e.g. the sink-composite batch) is
    // applied before the pass. Call once per frame, before endOffscreenFrame().
    void recordGather(QRhi *rhi, QRhiCommandBuffer *cb, QRhiTexture *sink,
                      const QVector<QPointF> &uvs, QRhiResourceUpdateBatch *preBatch);

    // After endOffscreenFrame(), fills `out` with the N gathered colours (in uv
    // order). Returns false if no gather was recorded / readback is empty.
    bool takeColors(QVector<QColor> &out);

private:
    bool ensureTarget(QRhi *rhi, const QSize &size);
    bool ensurePipeline(QRhi *rhi);
    bool ensureSrb(QRhi *rhi, QRhiTexture *sink);
    bool ensureVertexBuffer(QRhi *rhi, int vertexCount);

    QRhiTexture              *m_target = nullptr;
    QRhiTextureRenderTarget  *m_rt = nullptr;
    QRhiRenderPassDescriptor *m_rp = nullptr;
    QRhiGraphicsPipeline     *m_pipeline = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiSampler              *m_sampler = nullptr;
    QRhiBuffer               *m_vbuf = nullptr;

    QSize m_targetSize;         // packed WxH
    int   m_vbufCapacity = 0;   // vertices the buffer can hold
    QRhiTexture *m_lastSink = nullptr;

    int  m_pointCount = 0;      // N recorded this frame
    bool m_pending = false;     // a readback was queued
    class ReadbackHolder;
    ReadbackHolder *m_readback; // pImpl for QRhiReadbackResult (avoids rhi include in header)
};

} // namespace photon

#endif // PHOTON_CANVASDMXSAMPLER_H
