#include <rhi/qrhi.h>
#include <QThread>
#include <QCoreApplication>
#include <QColor>
#include <QFile>
#include <QDebug>
#include "basecanvasnode.h"
#include "rhi/rhicontext.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

namespace {
QShader loadShader(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
        return QShader::fromSerialized(f.readAll());
    qWarning() << "BaseCanvasNode: failed to load shader" << path;
    return QShader();
}
}

BaseCanvasNode::BaseCanvasNode(const QByteArray &id) : keira::Node(id)
{
}

BaseCanvasNode::~BaseCanvasNode()
{
    releaseAll();
}

bool BaseCanvasNode::ensureResources(QRhi *rhi, const QSize &size, const QVector<QRhiTexture *> &textures) const
{
    if (!m_ubuf) {
        m_ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, uniformSize());
        if (!m_ubuf->create()) {
            qWarning() << "BaseCanvasNode: uniform buffer create failed";
            return false;
        }
    }

    if (!textures.isEmpty() && !m_sampler) {
        m_sampler = rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                    QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_sampler->create())
            return false;
    }

    // Output texture + render target, recreated on resize (render pass descriptor
    // stays compatible — same format — so the pipeline survives a resize).
    if (!m_output || m_size != size) {
        delete m_rt;     m_rt = nullptr;
        delete m_output; m_output = nullptr;

        m_output = rhi->newTexture(QRhiTexture::RGBA8, size, 1,
                                   QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
        if (!m_output->create()) {
            qWarning() << "BaseCanvasNode: output texture create failed" << size;
            delete m_output;
            m_output = nullptr;
            return false;
        }

        QRhiColorAttachment colorAtt(m_output);
        QRhiTextureRenderTargetDescription rtDesc(colorAtt);
        m_rt = rhi->newTextureRenderTarget(rtDesc);
        if (!m_rp)
            m_rp = m_rt->newCompatibleRenderPassDescriptor();
        m_rt->setRenderPassDescriptor(m_rp);
        if (!m_rt->create())
            return false;

        m_size = size;
    }

    // Shader resource bindings: uniform buffer at 0, one sampled texture per input
    // at bindings 1..N. Inputs can change frame to frame, so rebuild when the
    // texture list changes (layout stays identical).
    if (!m_srb || m_lastTextures != textures) {
        delete m_srb;
        m_srb = rhi->newShaderResourceBindings();
        QVector<QRhiShaderResourceBinding> bindings;
        bindings.append(QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, m_ubuf));
        for (int i = 0; i < textures.size(); ++i)
            bindings.append(QRhiShaderResourceBinding::sampledTexture(
                1 + i, QRhiShaderResourceBinding::FragmentStage, textures[i], m_sampler));
        m_srb->setBindings(bindings.cbegin(), bindings.cend());
        if (!m_srb->create())
            return false;
        m_lastTextures = textures;
    }

    if (!m_pipeline) {
        const QShader vert = loadShader(QStringLiteral(":/canvas/shaders/fullscreen.vert.qsb"));
        const QShader frag = loadShader(QStringLiteral(":/canvas/shaders/")
                                        + QString::fromLatin1(fragmentShaderName()) + QStringLiteral(".frag.qsb"));
        if (!vert.isValid() || !frag.isValid())
            return false;

        m_pipeline = rhi->newGraphicsPipeline();
        m_pipeline->setShaderStages({
            { QRhiShaderStage::Vertex, vert },
            { QRhiShaderStage::Fragment, frag }
        });
        m_pipeline->setVertexInputLayout(QRhiVertexInputLayout());   // no vertex buffers
        m_pipeline->setShaderResourceBindings(m_srb);
        m_pipeline->setRenderPassDescriptor(m_rp);
        m_pipeline->setSampleCount(1);
        m_pipeline->setDepthTest(false);
        m_pipeline->setDepthWrite(false);
        m_pipeline->setCullMode(QRhiGraphicsPipeline::None);
        m_pipeline->setTopology(QRhiGraphicsPipeline::Triangles);
        if (!m_pipeline->create())
            return false;
    }

    return m_output && m_rt && m_rp && m_srb && m_pipeline;
}

void BaseCanvasNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto ctx = static_cast<RoutineEvaluationContext *>(t_context);
    if (!ctx->rhiContext || !ctx->rhiCommandBuffer)
        return;
    QRhi *rhi = ctx->rhiContext->rhi();
    const QSize size = ctx->canvasResolution;
    if (size.isEmpty())
        return;

    const QVector<RhiTextureData> ins = inputs();
    QVector<QRhiTexture *> textures;
    textures.reserve(ins.size());
    for (const auto &in : ins) {
        if (!in.texture) {
            // A required input is missing — produce nothing.
            if (m_outputParam)
                m_outputParam->setValue(RhiTextureData{});
            return;
        }
        textures.append(in.texture);
    }

    if (!ensureResources(rhi, size, textures))
        return;

    QByteArray ub(int(uniformSize()), '\0');
    writeUniforms(ub, size);

    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->updateDynamicBuffer(m_ubuf, 0, ub.size(), ub.constData());

    QRhiCommandBuffer *cb = ctx->rhiCommandBuffer;
    cb->beginPass(m_rt, QColor(0, 0, 0, 0), { 1.0f, 0 }, u);
    cb->setGraphicsPipeline(m_pipeline);
    cb->setViewport(QRhiViewport(0, 0, size.width(), size.height()));
    cb->setShaderResources(m_srb);
    cb->draw(3);
    cb->endPass();

    if (m_outputParam)
        m_outputParam->setValue(RhiTextureData{ m_output, size });
}

void BaseCanvasNode::releaseAll() const
{
    if (!m_output && !m_pipeline && !m_ubuf)
        return;

    // QRhi resources must be destroyed on the main thread. Detach the pointers and
    // (if we're off-thread, e.g. worker-thread graph editing) hand them off.
    QRhiGraphicsPipeline *pipe = m_pipeline;
    QRhiShaderResourceBindings *srb = m_srb;
    QRhiBuffer *ubuf = m_ubuf;
    QRhiSampler *samp = m_sampler;
    QRhiTextureRenderTarget *rt = m_rt;
    QRhiTexture *tex = m_output;
    QRhiRenderPassDescriptor *rp = m_rp;

    auto destroy = [pipe, srb, ubuf, samp, rt, tex, rp]() {
        delete pipe;   // references rp + srb
        delete srb;
        delete ubuf;
        delete samp;
        delete rt;     // references rp + tex
        delete tex;
        delete rp;
    };

    if (QThread::isMainThread())
        destroy();
    else
        QMetaObject::invokeMethod(QCoreApplication::instance(), destroy, Qt::QueuedConnection);

    m_pipeline = nullptr;
    m_srb = nullptr;
    m_ubuf = nullptr;
    m_sampler = nullptr;
    m_rt = nullptr;
    m_output = nullptr;
    m_rp = nullptr;
    m_lastTextures.clear();
    m_size = QSize();
}

} // namespace photon
