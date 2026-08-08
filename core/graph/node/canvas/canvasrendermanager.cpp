#include <QTimer>
#include "canvasrendermanager.h"
#include "canvassubgraphnode.h"
#include "sequence/canvaslayergroup.h"

namespace photon {

namespace {
// Set to the (single) manager while it lives. Nodes look it up here rather than
// through PhotonCore so headless tests without a manager degrade to inline render.
CanvasRenderManager *g_instance = nullptr;
}

CanvasRenderManager::CanvasRenderManager(RhiContext *rhiContext, QObject *parent)
    : QObject(parent), m_rhiContext(rhiContext)
{
    g_instance = this;

    // ~60 Hz. Renders only canvases flagged dirty by their last evaluate(), so an
    // idle canvas (no surface evaluating it) costs nothing but the poll.
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CanvasRenderManager::renderTick);
    m_timer->start(16);
}

CanvasRenderManager::~CanvasRenderManager()
{
    if (g_instance == this)
        g_instance = nullptr;
}

CanvasRenderManager *CanvasRenderManager::instance()
{
    return g_instance;
}

void CanvasRenderManager::registerCanvas(CanvasRenderable *node)
{
    QMutexLocker lock(&m_mutex);
    m_canvases.insert(node);
}

void CanvasRenderManager::unregisterCanvas(CanvasRenderable *node)
{
    // Blocks until any in-progress renderTick() releases the lock, so an
    // instance being destroyed (possibly on the worker thread) can't be freed
    // while the main thread is mid-render of it. Callers must unregister
    // before destroying state.
    QMutexLocker lock(&m_mutex);
    m_canvases.remove(node);
}

QVector<CanvasSubGraphNode *> CanvasRenderManager::canvases() const
{
    QMutexLocker lock(&m_mutex);
    QVector<CanvasSubGraphNode *> result;
    for (CanvasRenderable *r : m_canvases)
        if (auto *node = dynamic_cast<CanvasSubGraphNode *>(r))
            result.append(node);
    return result;
}

QVector<CanvasLayerGroup *> CanvasRenderManager::layerGroups() const
{
    QMutexLocker lock(&m_mutex);
    QVector<CanvasLayerGroup *> result;
    for (CanvasRenderable *r : m_canvases)
        if (auto *group = dynamic_cast<CanvasLayerGroup *>(r))
            result.append(group);
    return result;
}

bool CanvasRenderManager::isRegistered(CanvasRenderable *node) const
{
    QMutexLocker lock(&m_mutex);
    return m_canvases.contains(node);
}

void CanvasRenderManager::renderTick()
{
    // Hold the lock across the whole pass so register/unregister can't mutate the
    // set mid-iteration; the pointers stay valid for the render calls.
    QMutexLocker lock(&m_mutex);
    for (CanvasRenderable *node : m_canvases) {
        if (node->takeNeedsRender())
            node->renderMainThread();
    }
}

} // namespace photon
