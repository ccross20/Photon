#include <QTimer>
#include "canvasrendermanager.h"
#include "canvassubgraphnode.h"

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

void CanvasRenderManager::registerCanvas(CanvasSubGraphNode *node)
{
    QMutexLocker lock(&m_mutex);
    m_canvases.insert(node);
}

void CanvasRenderManager::unregisterCanvas(CanvasSubGraphNode *node)
{
    // Blocks until any in-progress renderTick() releases the lock, so a node being
    // destroyed (possibly on the worker thread) can't be freed while the main
    // thread is mid-render of it. Callers must unregister before destroying state.
    QMutexLocker lock(&m_mutex);
    m_canvases.remove(node);
}

QVector<CanvasSubGraphNode *> CanvasRenderManager::canvases() const
{
    QMutexLocker lock(&m_mutex);
    return QVector<CanvasSubGraphNode *>(m_canvases.cbegin(), m_canvases.cend());
}

bool CanvasRenderManager::isRegistered(CanvasSubGraphNode *node) const
{
    QMutexLocker lock(&m_mutex);
    return m_canvases.contains(node);
}

void CanvasRenderManager::renderTick()
{
    // Hold the lock across the whole pass so register/unregister can't mutate the
    // set mid-iteration; the pointers stay valid for the render calls.
    QMutexLocker lock(&m_mutex);
    for (CanvasSubGraphNode *node : m_canvases) {
        if (node->takeNeedsRender())
            node->renderMainThread();
    }
}

} // namespace photon
