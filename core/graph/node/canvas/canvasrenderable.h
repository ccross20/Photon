#ifndef PHOTON_CANVASRENDERABLE_H
#define PHOTON_CANVASRENDERABLE_H

namespace photon {

// Implemented by anything that owns a QRhi canvas texture and needs its
// rendering deferred to the main thread (QRhi/GL are main-thread only, but
// the thing driving the render - a node's evaluate(), a Layer's
// processChannels() - may run on keira's worker thread). CanvasRenderManager
// polls registered instances on a timer and calls renderMainThread() for
// ones flagged dirty by takeNeedsRender(). See [[canvas-gpu-pipeline]].
class CanvasRenderable
{
public:
    virtual ~CanvasRenderable() = default;

    // Consumes the dirty flag set by whatever last evaluated this instance.
    // Returns true if a render is due. Called by the manager on the main
    // thread. Const because implementations lazily build/mutate GPU state
    // behind `mutable` members (matching keira::Node::evaluate()'s const
    // contract, which CanvasSubGraphNode - a keira::Node - must also satisfy).
    virtual bool takeNeedsRender() const = 0;

    // Does the actual QRhi work (open frame, render, end frame). Main thread
    // only - never call this from a worker thread.
    virtual void renderMainThread() const = 0;
};

} // namespace photon

#endif // PHOTON_CANVASRENDERABLE_H
