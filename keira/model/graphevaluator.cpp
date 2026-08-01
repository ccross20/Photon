#include "graphevaluator.h"
#include "clockcontroller.h"
#include "graph.h"
#include "node.h"
#include <QElapsedTimer>
#include <QThread>
#include <QTimer>

namespace keira {

static constexpr qint64 FrameBudgetMs = 1000 / 44; // ~22ms at 44Hz

// ─────────────────────────────────────────────────────────────────────────────
// EvalWorker — lives on the eval thread, owns the clock and runs the graph
// ─────────────────────────────────────────────────────────────────────────────

class EvalWorker : public QObject
{
    Q_OBJECT
public:
    using ContextFactory   = GraphEvaluator::ContextFactory;
    using PostEvalCallback = GraphEvaluator::PostEvalCallback;

    explicit EvalWorker(Graph *graph, ContextFactory factory, PostEvalCallback postEval)
        : m_graph(graph)
        , m_contextFactory(std::move(factory))
        , m_postEvalCallback(std::move(postEval))
    {}

    ClockController *clock() { return &m_clock; }

public slots:
    void start()
    {
        // Timer is created here so it belongs to the eval thread.
        // QTimer member variables created in the constructor live on the main
        // thread and cannot be started from a different thread.
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &EvalWorker::tick);
        m_frameTimer.start();
        m_timer->start(FrameBudgetMs);
    }

    void stop()
    {
        if (m_timer)
            m_timer->stop();
    }

    void setGraph(keira::Graph *graph)
    {
        m_graph = graph;
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        // Keep the timer running even when disabled so queued structural edits
        // (add/remove node, connect) still drain and apply — the editor must stay
        // usable with auto-evaluate off. tick() skips only the evaluation itself.
        if (m_timer && !m_timer->isActive())
            m_timer->start(FrameBudgetMs);
    }

    void setContextFactory(ContextFactory factory)
    {
        m_contextFactory = std::move(factory);
    }

    void setPostEvalCallback(PostEvalCallback callback)
    {
        m_postEvalCallback = std::move(callback);
    }

private slots:
    void tick()
    {
        const double delta = m_frameTimer.elapsed() / 1000.0;
        m_frameTimer.restart();

        m_clock.tick(delta);

        if (!m_graph)
            return;

        // Apply queued structural edits every tick, even when paused, so the
        // editor stays usable with auto-evaluate off.
        m_graph->drainCommandQueue();

        if (!m_enabled)
            return;

        m_graph->prepForEvaluation();

        const FrameTime frameTime = m_clock.currentTime();

        EvaluationContext *ctx = nullptr;
        if (m_contextFactory)
            ctx = m_contextFactory(frameTime);
        else
            ctx = new EvaluationContext;

        ctx->time = frameTime;

        m_graph->evaluate(ctx);
        m_graph->markClean();

        delete ctx;

        if (m_postEvalCallback)
            m_postEvalCallback();

        emit evaluationComplete();
    }

signals:
    void evaluationComplete();

private:
    ClockController  m_clock;
    Graph           *m_graph            = nullptr;
    QTimer          *m_timer            = nullptr;
    QElapsedTimer    m_frameTimer;
    bool             m_enabled          = true;
    ContextFactory   m_contextFactory;
    PostEvalCallback m_postEvalCallback;
};

// ─────────────────────────────────────────────────────────────────────────────
// GraphEvaluator
// ─────────────────────────────────────────────────────────────────────────────

class GraphEvaluator::Impl
{
public:
    QThread    *thread  = nullptr;
    EvalWorker *worker  = nullptr;
    bool        enabled = true;
};

GraphEvaluator::GraphEvaluator(Graph *t_graph,
                               ContextFactory factory,
                               PostEvalCallback postEval,
                               QObject *parent)
    : QObject{parent}, m_impl(new Impl)
{
    m_impl->worker = new EvalWorker(t_graph, std::move(factory), std::move(postEval));
    m_impl->thread = new QThread(this);

    m_impl->worker->moveToThread(m_impl->thread);

    connect(m_impl->thread, &QThread::finished, m_impl->worker, &QObject::deleteLater);
    connect(m_impl->worker, &EvalWorker::evaluationComplete,
            this, &GraphEvaluator::evaluationComplete);

    m_impl->thread->start();

    // invokeMethod queues start() on the worker's thread once the event loop
    // is running — more reliable than connecting to QThread::started, whose
    // emit context (new thread vs. caller thread) is implementation-defined.
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker]() { w->start(); });
}

GraphEvaluator::~GraphEvaluator()
{
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker]() { w->stop(); });
    m_impl->thread->quit();
    m_impl->thread->wait();
    delete m_impl;
}

bool GraphEvaluator::isEnabled() const
{
    return m_impl->enabled;
}

FrameTime GraphEvaluator::currentTime() const
{
    return m_impl->worker->clock()->currentTime();
}

void GraphEvaluator::setContextFactory(ContextFactory factory)
{
    QMetaObject::invokeMethod(m_impl->worker,
        [w = m_impl->worker, f = std::move(factory)]() mutable {
            w->setContextFactory(std::move(f));
        });
}

void GraphEvaluator::setPostEvalCallback(PostEvalCallback callback)
{
    QMetaObject::invokeMethod(m_impl->worker,
        [w = m_impl->worker, cb = std::move(callback)]() mutable {
            w->setPostEvalCallback(std::move(cb));
        });
}

void GraphEvaluator::setGraph(keira::Graph *t_graph)
{
    // Swap the graph synchronously: block until the worker has applied it (and
    // finished any in-progress tick). Callers rely on this so they can safely
    // delete the previously-evaluated graph the moment setGraph() returns — an
    // async post would let the eval thread keep ticking the old (about-to-be-
    // freed) graph. Guard against being called from the eval thread itself,
    // where a blocking queued call would deadlock.
    if (QThread::currentThread() == m_impl->thread) {
        m_impl->worker->setGraph(t_graph);
        return;
    }
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker, t_graph]() {
        w->setGraph(t_graph);
    }, Qt::BlockingQueuedConnection);
}

void GraphEvaluator::setIsEnabled(bool t_value)
{
    if (m_impl->enabled == t_value)
        return;
    m_impl->enabled = t_value;
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker, t_value]() {
        w->setEnabled(t_value);
    });
}

void GraphEvaluator::setBpm(double bpm)
{
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker, bpm]() {
        w->clock()->setBpm(bpm);
    });
}

void GraphEvaluator::setBeatsPerBar(int beatsPerBar)
{
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker, beatsPerBar]() {
        w->clock()->setBeatsPerBar(beatsPerBar);
    });
}

void GraphEvaluator::tap()
{
    QMetaObject::invokeMethod(m_impl->worker, [w = m_impl->worker]() {
        w->clock()->tap();
    });
}

} // namespace keira

// Required for Q_OBJECT defined in this .cpp file
#include "graphevaluator.moc"
