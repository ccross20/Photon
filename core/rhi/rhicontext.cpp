#include <rhi/qrhi.h>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QColor>
#include <QList>
#include <QDebug>
#include "rhicontext.h"

namespace photon {

class RhiContext::Impl
{
public:
    QOffscreenSurface *fallbackSurface = nullptr;
    QOpenGLContext *context = nullptr;   // shares globalShareContext; owned by QRhi import
    QRhi *rhi = nullptr;

    // Pooled render textures. A pointer sits in exactly one of the lists.
    struct PooledTexture {
        QRhiTexture *texture;
        QSize size;
    };
    QList<PooledTexture> free;
    QList<PooledTexture> inUse;
    // Pooled textures are torn down explicitly in ~RhiContext (they must go while
    // the QRhi is still alive), so this class has no destructor of its own.
};

RhiContext::RhiContext() : m_impl(new Impl)
{
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);

    m_impl->fallbackSurface = QRhiGles2InitParams::newFallbackSurface(fmt);

    // Our own GL context, sharing the app-wide share context. This is the same
    // sharing model Canvas already uses (see canvas.cpp), so texture ids created
    // here are visible to other contexts in the share group.
    m_impl->context = new QOpenGLContext;
    m_impl->context->setShareContext(QOpenGLContext::globalShareContext());
    m_impl->context->setFormat(fmt);
    if (!m_impl->context->create()) {
        qWarning() << "RhiContext: failed to create shared QOpenGLContext";
        delete m_impl->context;
        m_impl->context = nullptr;
        return;
    }

    QRhiGles2InitParams params;
    params.format = fmt;
    params.fallbackSurface = m_impl->fallbackSurface;

    // Import our shared context rather than letting QRhi create its own, so the
    // device lives in the global share group.
    QRhiGles2NativeHandles importDevice;
    importDevice.context = m_impl->context;

    m_impl->rhi = QRhi::create(QRhi::OpenGLES2, &params, {}, &importDevice);
    if (!m_impl->rhi)
        qWarning() << "RhiContext: failed to create offscreen QRhi (OpenGL backend)";
    else
        qInfo() << "RhiContext: offscreen QRhi created —" << m_impl->rhi->backendName()
                << m_impl->rhi->driverInfo().deviceName;
}

RhiContext::~RhiContext()
{
    // Pooled textures must go while the QRhi is still alive.
    for (const auto &p : m_impl->free)  delete p.texture;
    for (const auto &p : m_impl->inUse) delete p.texture;
    m_impl->free.clear();
    m_impl->inUse.clear();

    delete m_impl->rhi;             // releases the imported context's GPU resources
    delete m_impl->context;         // we own the QOpenGLContext (imported, not created by QRhi)
    delete m_impl->fallbackSurface;
    delete m_impl;
}

bool RhiContext::isValid() const
{
    return m_impl->rhi != nullptr;
}

QRhi *RhiContext::rhi() const
{
    return m_impl->rhi;
}

QRhiTexture *RhiContext::acquireTexture(const QSize &size)
{
    if (!m_impl->rhi)
        return nullptr;

    // Reuse a free texture of the exact size.
    for (int i = 0; i < m_impl->free.size(); ++i) {
        if (m_impl->free.at(i).size == size) {
            Impl::PooledTexture p = m_impl->free.takeAt(i);
            m_impl->inUse.append(p);
            return p.texture;
        }
    }

    QRhiTexture *tex = m_impl->rhi->newTexture(
        QRhiTexture::RGBA8, size, 1,
        QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    if (!tex->create()) {
        qWarning() << "RhiContext::acquireTexture — create failed for" << size;
        delete tex;
        return nullptr;
    }

    m_impl->inUse.append({ tex, size });
    return tex;
}

void RhiContext::releaseTexture(QRhiTexture *texture)
{
    if (!texture)
        return;

    for (int i = 0; i < m_impl->inUse.size(); ++i) {
        if (m_impl->inUse.at(i).texture == texture) {
            m_impl->free.append(m_impl->inUse.takeAt(i));
            return;
        }
    }
    qWarning() << "RhiContext::releaseTexture — texture not from this pool (or double-released)";
}

bool RhiContext::selfTest()
{
    QRhi *rhi = m_impl->rhi;
    if (!rhi) {
        qWarning() << "RhiContext::selfTest — no device";
        return false;
    }

    const QSize size(16, 16);
    const QColor clearColor = QColor::fromRgbF(0.20f, 0.40f, 0.80f, 1.0f);

    QScopedPointer<QRhiTexture> tex(rhi->newTexture(
        QRhiTexture::RGBA8, size, 1,
        QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!tex->create()) {
        qWarning() << "RhiContext::selfTest — texture create failed";
        return false;
    }

    QRhiTextureRenderTargetDescription rtDesc(QRhiColorAttachment(tex.data()));
    QScopedPointer<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    QScopedPointer<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.data());
    if (!rt->create()) {
        qWarning() << "RhiContext::selfTest — render target create failed";
        return false;
    }

    QRhiCommandBuffer *cb = nullptr;
    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb) {
        qWarning() << "RhiContext::selfTest — beginOffscreenFrame failed";
        return false;
    }

    // Clear the target, then read it back within the same offscreen frame.
    cb->beginPass(rt.data(), clearColor, { 1.0f, 0 });

    QRhiReadbackResult readback;
    bool readbackDone = false;
    readback.completed = [&readbackDone]() { readbackDone = true; };
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture(QRhiReadbackDescription(tex.data()), &readback);
    cb->endPass(u);

    rhi->endOffscreenFrame();   // blocks until the GPU is done for offscreen frames

    if (!readbackDone || readback.data.isEmpty()) {
        qWarning() << "RhiContext::selfTest — readback produced no data";
        return false;
    }

    // Compare the first texel against the clear colour. GL readback is RGBA8,
    // bottom-up; the clear colour is uniform so orientation doesn't matter here.
    const uchar *p = reinterpret_cast<const uchar *>(readback.data.constData());
    const int er = qRound(clearColor.redF()   * 255.0);
    const int eg = qRound(clearColor.greenF() * 255.0);
    const int eb = qRound(clearColor.blueF()  * 255.0);
    const int tol = 2;
    const bool ok = qAbs(int(p[0]) - er) <= tol
                 && qAbs(int(p[1]) - eg) <= tol
                 && qAbs(int(p[2]) - eb) <= tol;

    if (ok)
        qInfo() << "RhiContext::selfTest — PASS (texel"
                << p[0] << p[1] << p[2] << p[3] << ")";
    else
        qWarning() << "RhiContext::selfTest — FAIL: expected" << er << eg << eb
                   << "got" << p[0] << p[1] << p[2] << p[3];

    return ok;
}

} // namespace photon
