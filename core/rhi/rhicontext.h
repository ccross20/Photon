#ifndef PHOTON_RHICONTEXT_H
#define PHOTON_RHICONTEXT_H

#include <QSize>
#include "photon-global.h"

class QRhi;
class QRhiTexture;
class QOffscreenSurface;
class QOpenGLContext;

namespace photon {

// Core-owned, headless QRhi device for the canvas system.
//
// Unlike the visualizer (where each RhiWindow owns a QRhi bound to a window +
// swapchain), the canvas pipeline must render with no window visible — DMX
// output has to work whether or not a preview panel is open. So this owns a
// single offscreen QRhi, created through rhiBackend (Metal on macOS, OpenGL
// elsewhere).
//
// This is the first device created in the process, and the other two (the
// visualizer window and the canvas preview window) are built to interoperate
// with it: on Metal they import the MTLDevice it seeds, on OpenGL it lives in
// the global share group. Either way a preview window's separate QRhi can
// import a canvas texture by its native handle. See core/rhi/rhibackend.h.
class PHOTONCORE_EXPORT RhiContext
{
public:
    RhiContext();
    ~RhiContext();

    // True once the QRhi device was created successfully.
    bool isValid() const;

    QRhi *rhi() const;

    // Texture pool for the canvas node graph. A node acquires an output texture
    // (colour render target, also readable as a transfer source), renders into
    // it, and passes the handle downstream via RhiTextureParameter; once the
    // frame's consumers are done it's released back for reuse. Pooling avoids
    // per-frame GPU allocation as the graph is re-evaluated. Textures are RGBA8
    // with usage RenderTarget | UsedAsTransferSource.
    //
    // Handles are owned by the pool — never delete them; release them instead.
    // Both calls must run on the app/GUI thread (single QRhi, no locking).
    QRhiTexture *acquireTexture(const QSize &size);
    void releaseTexture(QRhiTexture *texture);

    // Renders a known clear colour into a throwaway offscreen QRhiTexture and
    // reads it back, verifying the pipeline round-trips. Logs the outcome and
    // returns true on success. Used by the Phase 1 spike to validate that a
    // core-owned offscreen device works alongside the visualizer.
    bool selfTest();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_RHICONTEXT_H
