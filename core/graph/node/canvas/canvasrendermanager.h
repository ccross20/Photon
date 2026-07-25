#ifndef PHOTON_CANVASRENDERMANAGER_H
#define PHOTON_CANVASRENDERMANAGER_H

#include <QObject>
#include <QSet>
#include <QVector>
#include <QMutex>
#include "photon-global.h"

class QTimer;

namespace photon {

class CanvasSubGraphNode;
class RhiContext;

// Drives canvas GPU rendering on the MAIN thread.
//
// Surface/bus graphs evaluate on a worker thread (keira's EvalWorker), but QRhi
// and its GL context are thread-affine and must only be touched on the GUI
// thread. So a CanvasSubGraphNode's worker-thread evaluate() only snapshots its
// inputs and flags itself dirty; this manager — living on the main thread, owned
// by PhotonCore — polls registered canvases on a timer and calls renderMainThread()
// for the dirty ones, on the main thread.
//
// A single static instance is exposed so nodes can find it without depending on
// PhotonCore (keeps headless tests, e.g. rhi-spike, working: with no manager,
// a node renders inline on whatever thread evaluates it). See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasRenderManager : public QObject
{
    Q_OBJECT
public:
    explicit CanvasRenderManager(RhiContext *rhiContext, QObject *parent = nullptr);
    ~CanvasRenderManager();

    static CanvasRenderManager *instance();

    // The shared main-thread device canvas nodes render through.
    RhiContext *rhiContext() const { return m_rhiContext; }

    void registerCanvas(CanvasSubGraphNode *);
    void unregisterCanvas(CanvasSubGraphNode *);

    // Snapshot of the currently registered canvases (for a preview picker etc.).
    QVector<CanvasSubGraphNode *> canvases() const;
    // Whether a canvas is still live/registered — validate a held pointer before use.
    bool isRegistered(CanvasSubGraphNode *) const;

private slots:
    void renderTick();

private:
    RhiContext *m_rhiContext = nullptr;
    QTimer *m_timer = nullptr;
    mutable QMutex m_mutex;                // guards m_canvases against register/unregister off-thread
    QSet<CanvasSubGraphNode *> m_canvases;
};

} // namespace photon

#endif // PHOTON_CANVASRENDERMANAGER_H
