#include <cmath>
#include <rhi/qrhi.h>
#include <QThread>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "canvasdmxsampler.h"

namespace photon {

namespace {
constexpr int kMaxWidth = 4096;   // packed-target width cap (fits texture limits)

QShader loadShader(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());
    qWarning() << "CanvasDmxSampler: failed to load shader" << path;
    return QShader();
}
}

class CanvasDmxSampler::ReadbackHolder
{
public:
    QRhiReadbackResult result;
    bool done = false;
};

CanvasDmxSampler::CanvasDmxSampler() : m_readback(new ReadbackHolder)
{
}

CanvasDmxSampler::~CanvasDmxSampler()
{
    // QRhi resources must be destroyed on the main thread; defer if not on it
    // (the owning node may be torn down on the worker thread).
    QRhiGraphicsPipeline *pipe = m_pipeline;
    QRhiShaderResourceBindings *srb = m_srb;
    QRhiSampler *samp = m_sampler;
    QRhiBuffer *vbuf = m_vbuf;
    QRhiTextureRenderTarget *rt = m_rt;
    QRhiTexture *tex = m_target;
    QRhiRenderPassDescriptor *rp = m_rp;
    auto destroy = [pipe, srb, samp, vbuf, rt, tex, rp]() {
        delete pipe; delete srb; delete samp; delete vbuf; delete rt; delete tex; delete rp;
    };
    if (pipe || tex) {
        if (QThread::isMainThread())
            destroy();
        else
            QMetaObject::invokeMethod(QCoreApplication::instance(), destroy, Qt::QueuedConnection);
    }
    delete m_readback;
}

bool CanvasDmxSampler::ensureTarget(QRhi *rhi, const QSize &size)
{
    if (m_target && m_targetSize == size)
        return true;

    delete m_rt;     m_rt = nullptr;
    delete m_target; m_target = nullptr;

    m_target = rhi->newTexture(QRhiTexture::RGBA8, size, 1,
                               QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    if (!m_target->create()) {
        delete m_target; m_target = nullptr;
        return false;
    }
    QRhiColorAttachment colorAtt(m_target);
    QRhiTextureRenderTargetDescription rtDesc(colorAtt);
    m_rt = rhi->newTextureRenderTarget(rtDesc);
    if (!m_rp)
        m_rp = m_rt->newCompatibleRenderPassDescriptor();
    m_rt->setRenderPassDescriptor(m_rp);
    if (!m_rt->create())
        return false;
    m_targetSize = size;
    return true;
}

bool CanvasDmxSampler::ensureSrb(QRhi *rhi, QRhiTexture *sink)
{
    if (!m_sampler) {
        m_sampler = rhi->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                    QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_sampler->create())
            return false;
    }
    if (!m_srb || m_lastSink != sink) {
        delete m_srb;
        m_srb = rhi->newShaderResourceBindings();
        m_srb->setBindings({
            QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, sink, m_sampler)
        });
        if (!m_srb->create())
            return false;
        m_lastSink = sink;
    }
    return true;
}

bool CanvasDmxSampler::ensurePipeline(QRhi *rhi)
{
    if (m_pipeline)
        return true;

    const QShader vert = loadShader(QStringLiteral(":/canvas/shaders/gather.vert.qsb"));
    const QShader frag = loadShader(QStringLiteral(":/canvas/shaders/gather.frag.qsb"));
    if (!vert.isValid() || !frag.isValid())
        return false;

    m_pipeline = rhi->newGraphicsPipeline();
    m_pipeline->setTopology(QRhiGraphicsPipeline::Points);
    m_pipeline->setShaderStages({
        { QRhiShaderStage::Vertex, vert },
        { QRhiShaderStage::Fragment, frag }
    });
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ { 4 * sizeof(float) } });          // clipPos(vec2) + uv(vec2)
    inputLayout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) }
    });
    m_pipeline->setVertexInputLayout(inputLayout);
    m_pipeline->setShaderResourceBindings(m_srb);
    m_pipeline->setRenderPassDescriptor(m_rp);
    m_pipeline->setSampleCount(1);
    m_pipeline->setDepthTest(false);
    m_pipeline->setDepthWrite(false);
    m_pipeline->setCullMode(QRhiGraphicsPipeline::None);
    return m_pipeline->create();
}

bool CanvasDmxSampler::ensureVertexBuffer(QRhi *rhi, int vertexCount)
{
    if (m_vbuf && m_vbufCapacity >= vertexCount)
        return true;
    delete m_vbuf;
    m_vbufCapacity = qMax(vertexCount, 64);
    m_vbuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                            quint32(m_vbufCapacity) * 4 * sizeof(float));
    return m_vbuf->create();
}

void CanvasDmxSampler::recordGather(QRhi *rhi, QRhiCommandBuffer *cb, QRhiTexture *sink,
                                    const QVector<QPointF> &uvs, QRhiResourceUpdateBatch *preBatch)
{
    m_pending = false;
    m_pointCount = 0;
    const int n = uvs.size();
    if (n == 0 || !sink) {
        if (preBatch)
            cb->resourceUpdate(preBatch);
        return;
    }

    const int w = qMin(n, kMaxWidth);
    const int h = (n + w - 1) / w;

    if (!ensureTarget(rhi, QSize(w, h)) || !ensureSrb(rhi, sink)
            || !ensurePipeline(rhi) || !ensureVertexBuffer(rhi, n)) {
        if (preBatch)
            cb->resourceUpdate(preBatch);
        return;
    }

    // Vertex data: place point i at packed texel (i%w, i/w); sample uv (with the
    // canvas' bottom-up flip so it matches on-screen/CPU sampling).
    QVector<float> verts(n * 4);
    for (int i = 0; i < n; ++i) {
        const int px = i % w;
        const int py = i / w;
        verts[i * 4 + 0] = (float(px) + 0.5f) / float(w) * 2.0f - 1.0f;
        verts[i * 4 + 1] = (float(py) + 0.5f) / float(h) * 2.0f - 1.0f;
        verts[i * 4 + 2] = float(uvs[i].x());
        verts[i * 4 + 3] = 1.0f - float(uvs[i].y());
    }
    QRhiResourceUpdateBatch *batch = preBatch ? preBatch : rhi->nextResourceUpdateBatch();
    batch->updateDynamicBuffer(m_vbuf, 0, quint32(n) * 4 * sizeof(float), verts.constData());

    cb->beginPass(m_rt, QColor(0, 0, 0, 0), { 1.0f, 0 }, batch);
    cb->setGraphicsPipeline(m_pipeline);
    cb->setViewport(QRhiViewport(0, 0, w, h));
    cb->setShaderResources(m_srb);
    const QRhiCommandBuffer::VertexInput vbufBinding(m_vbuf, 0);
    cb->setVertexInput(0, 1, &vbufBinding);
    cb->draw(quint32(n));
    cb->endPass();

    m_readback->done = false;
    m_readback->result = QRhiReadbackResult();
    m_readback->result.completed = [this]() { m_readback->done = true; };
    QRhiResourceUpdateBatch *rb = rhi->nextResourceUpdateBatch();
    rb->readBackTexture(QRhiReadbackDescription(m_target), &m_readback->result);
    cb->resourceUpdate(rb);

    m_pointCount = n;
    m_pending = true;
}

bool CanvasDmxSampler::takeColors(QVector<QColor> &out)
{
    if (!m_pending || !m_readback->done || m_readback->result.data.isEmpty())
        return false;

    const uchar *p = reinterpret_cast<const uchar *>(m_readback->result.data.constData());
    const int available = m_readback->result.data.size() / 4;
    const int n = qMin(m_pointCount, available);
    out.resize(n);
    for (int i = 0; i < n; ++i)
        out[i] = QColor(p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);

    m_pending = false;
    return true;
}

} // namespace photon
