#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QSurface>
#include <QDebug>
#include <rhi/qrhi.h>
#include "rhibackend.h"

namespace photon {
namespace rhiBackend {

QRhi::Implementation api()
{
#if defined(Q_OS_MACOS)
    return QRhi::Metal;
#else
    return QRhi::OpenGLES2;
#endif
}

const char *apiName()
{
#if defined(Q_OS_MACOS)
    return "Metal";
#else
    return "OpenGL";
#endif
}

#if defined(Q_OS_MACOS)
// The MTLDevice every QRhi in the process shares, seeded by the first one
// created here. Deliberately a weak pointer: it belongs to that first QRhi,
// which QRhi releases with itself.
//
// That first device is always RhiContext's. PhotonCore::init() creates it
// eagerly (via rhiContext(), for CanvasRenderManager) before main() calls
// gui()->launchInterface(), so no panel - and so no window-backed device - can
// exist yet; and ~PhotonCore deletes it after them. If that ordering is ever
// broken, a window device could seed this and then be destroyed on panel
// close, leaving every later import pointing at a freed device.
static MTLDevice *s_sharedDevice = nullptr;
#endif

QSurface::SurfaceType surfaceType()
{
#if defined(Q_OS_MACOS)
    return QSurface::MetalSurface;
#else
    return QSurface::OpenGLSurface;
#endif
}

Result create(const QSurfaceFormat &format, QWindow *window, bool shareGL)
{
    Result result;

#if defined(Q_OS_MACOS)
    Q_UNUSED(format)
    Q_UNUSED(shareGL)
    // Metal takes the target window from the swapchain, not from init params.
    Q_UNUSED(window)

    QRhiMetalInitParams params;

    if (s_sharedDevice) {
        // Import the process-wide device. cmdQueue is left null on purpose:
        // QRhi then makes its own queue, which is what we want - each device
        // submits independently, and textures stay shareable because the
        // MTLDevice is the same.
        QRhiMetalNativeHandles importDevice;
        importDevice.dev = s_sharedDevice;
        result.rhi = QRhi::create(QRhi::Metal, &params, {}, &importDevice);
    } else {
        result.rhi = QRhi::create(QRhi::Metal, &params);
        if (result.rhi) {
            const auto *h = static_cast<const QRhiMetalNativeHandles *>(result.rhi->nativeHandles());
            s_sharedDevice = h ? h->dev : nullptr;
            if (!s_sharedDevice)
                qWarning() << "rhiBackend: Metal device created but no native handle;"
                           << "later devices will not share it";
        }
    }

    if (!result.rhi)
        qWarning() << "rhiBackend: failed to create Metal QRhi";

    return result;
#else
    result.fallbackSurface = QRhiGles2InitParams::newFallbackSurface(format);

    QRhiGles2InitParams params;
    params.format = format;
    params.fallbackSurface = result.fallbackSurface;
    params.window = window;

    if (shareGL) {
        // Our own context in the app-wide share group, so texture ids created
        // on this device are visible to the others.
        result.context = new QOpenGLContext;
        result.context->setShareContext(QOpenGLContext::globalShareContext());
        result.context->setFormat(format);
        if (!result.context->create()) {
            qWarning() << "rhiBackend: failed to create shared QOpenGLContext";
            delete result.context;
            result.context = nullptr;
            delete result.fallbackSurface;
            result.fallbackSurface = nullptr;
            return result;
        }

        QRhiGles2NativeHandles importDevice;
        importDevice.context = result.context;
        result.rhi = QRhi::create(QRhi::OpenGLES2, &params, {}, &importDevice);
    } else {
        result.rhi = QRhi::create(QRhi::OpenGLES2, &params);
    }

    if (!result.rhi) {
        qWarning() << "rhiBackend: failed to create OpenGL QRhi";
        // The caller has no handle to these yet, so clean up here rather than
        // leaving it to a teardown path that will bail on the null QRhi.
        delete result.context;
        result.context = nullptr;
        delete result.fallbackSurface;
        result.fallbackSurface = nullptr;
    }

    return result;
#endif
}

} // namespace rhiBackend
} // namespace photon
