#ifndef PHOTON_CANVASRENDERMANAGER_H
#define PHOTON_CANVASRENDERMANAGER_H

#include <QObject>
#include <QSet>
#include <QVector>
#include <QMutex>
#include "photon-global.h"
#include "canvasrenderable.h"

class QTimer;

namespace photon {

class RhiContext;
class CanvasSubGraphNode;
class CanvasLayerGroup;

// Drives canvas GPU rendering on the MAIN thread.
//
// Surface/bus graphs evaluate on a worker thread (keira's EvalWorker), but QRhi
// and its GL context are thread-affine and must only be touched on the GUI
// thread. So a CanvasRenderable's worker-thread evaluate()/processChannels()
// only snapshots its inputs and flags itself dirty; this manager — living on
// the main thread, owned by PhotonCore — polls registered instances on a timer
// and calls renderMainThread() for the dirty ones, on the main thread.
//
// Any CanvasRenderable can register - CanvasSubGraphNode (a bus/surface node)
// and CanvasLayerGroup (a sequence layer, unrelated class hierarchy) both do -
// this manager doesn't need to know which.
//
// A single static instance is exposed so callers can find it without depending
// on PhotonCore (keeps headless tests, e.g. rhi-spike, working: with no
// manager, a CanvasSubGraphNode renders inline on whatever thread evaluates
// it). See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasRenderManager : public QObject
{
    Q_OBJECT
public:
    explicit CanvasRenderManager(RhiContext *rhiContext, QObject *parent = nullptr);
    ~CanvasRenderManager();

    static CanvasRenderManager *instance();

    // The shared main-thread device canvas nodes render through.
    RhiContext *rhiContext() const { return m_rhiContext; }

    void registerCanvas(CanvasRenderable *);
    void unregisterCanvas(CanvasRenderable *);

    // Snapshot of the currently registered CanvasSubGraphNodes (for the canvas
    // preview picker, which lists their Output nodes - a CanvasSubGraphNode-
    // specific concept). Other CanvasRenderable kinds (e.g. CanvasLayerGroup)
    // are filtered out, not just anything registered.
    QVector<CanvasSubGraphNode *> canvases() const;
    // Snapshot of the currently registered CanvasLayerGroups (for the canvas
    // preview picker, which also lists each group's own sink alongside Output
    // node textures).
    QVector<CanvasLayerGroup *> layerGroups() const;
    // Whether an instance is still live/registered — validate a held pointer before use.
    bool isRegistered(CanvasRenderable *) const;

private slots:
    void renderTick();

private:
    RhiContext *m_rhiContext = nullptr;
    QTimer *m_timer = nullptr;
    mutable QMutex m_mutex;                // guards m_canvases against register/unregister off-thread
    QSet<CanvasRenderable *> m_canvases;
};

} // namespace photon

#endif // PHOTON_CANVASRENDERMANAGER_H
