#ifndef PHOTON_RHIBACKEND_H
#define PHOTON_RHIBACKEND_H

#include <rhi/qrhi.h>
#include <QSurface>
#include "photon-global.h"

class QOffscreenSurface;
class QOpenGLContext;
class QSurfaceFormat;
class QWindow;

namespace photon {

// Process-wide choice of graphics API, plus the plumbing that keeps several
// QRhi instances interoperable.
//
// The app runs three QRhi devices: the core-owned offscreen canvas device
// (RhiContext), the visualizer window (RhiWindow) and the canvas preview
// window (CanvasPreviewWindow). Canvas textures are produced on the first and
// consumed by the third, so the devices have to be able to see each other's
// textures.
//
// On OpenGL that comes from the global share group (Qt::AA_ShareOpenGLContexts,
// set in main()). Metal has no equivalent: an MTLTexture is shareable across
// command queues, but only within one MTLDevice. So the first QRhi created here
// caches its MTLDevice and every later one imports it.
namespace rhiBackend {

// Metal on macOS, OpenGL everywhere else. Windows and Linux are deliberately
// left on OpenGL - the GL path is what they have always run.
PHOTONCORE_EXPORT QRhi::Implementation api();

// Human-readable backend name, for logging.
PHOTONCORE_EXPORT const char *apiName();

// The QWindow surface type a swapchain-backed device needs. QRhi refuses to
// build a swapchain on a mismatched window ("QMetalSwapChain only supports
// MetalSurface windows"), so every QWindow that will host a QRhi must call
// setSurfaceType() with this before it is created.
PHOTONCORE_EXPORT QSurface::SurfaceType surfaceType();

struct Result
{
    QRhi *rhi = nullptr;

    // OpenGL backend only; both null on Metal. Owned by the caller and must be
    // destroyed *after* the QRhi - its cleanup needs a current context.
    QOffscreenSurface *fallbackSurface = nullptr;
    QOpenGLContext    *context         = nullptr;   // only set when shareGL was true
};

// Creates a QRhi on the app's backend.
//
// `window` is the target for a swapchain-backed device, or null for an
// offscreen one. `format` and `shareGL` likewise matter only to the OpenGL
// backend: when `shareGL` is true the GL QRhi imports a QOpenGLContext placed
// in the global share group - what RhiContext needs so its canvas textures are
// visible to the preview window's separate device; when false QRhi creates its
// own context. All three are no-ops on Metal, where the swapchain carries the
// window and device sharing is handled here automatically.
//
// Must be called on the GUI thread.
PHOTONCORE_EXPORT Result create(const QSurfaceFormat &format,
                                QWindow *window = nullptr,
                                bool shareGL = false);

} // namespace rhiBackend
} // namespace photon

#endif // PHOTON_RHIBACKEND_H
