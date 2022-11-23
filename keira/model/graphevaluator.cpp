#include <QTimer>
#include "graphevaluator.h"
#include "graph.h"
#include "node.h"

namespace keira {

class GraphEvaluator::Impl
{
public:
    QTimer timer;
    Graph *graph;
    bool enabled = true;
};

GraphEvaluator::GraphEvaluator(Graph *t_graph, QObject *parent)
    : QObject{parent},m_impl(new Impl)
{
    m_impl->timer.setSingleShot(true);
    m_impl->graph = t_graph;

    connect(&m_impl->timer, &QTimer::timeout, this, &GraphEvaluator::timeout);
    if(m_impl->graph)
    {
        EvaluationContext context;
        connect(t_graph, &Graph::dirtyStateChanged, this, &GraphEvaluator::graphUpdated);
        if(t_graph->isDirty())
        {
            t_graph->evaluate(&context);
            t_graph->markClean();
        }
    }


}

GraphEvaluator::~GraphEvaluator()
{
    disconnect(m_impl->graph, &Graph::dirtyStateChanged, this, &GraphEvaluator::graphUpdated);
    delete m_impl;
}

void GraphEvaluator::setIsEnabled(bool t_value)
{
    if(m_impl->enabled == t_value)
        return;
    m_impl->enabled = t_value;

    if(m_impl->enabled)
        m_impl->timer.start(10);
}

bool GraphEvaluator::isEnabled() const
{
    return m_impl->enabled;
}

void GraphEvaluator::setGraph(keira::Graph* t_graph)
{
    disconnect(m_impl->graph, &Graph::dirtyStateChanged, this, &GraphEvaluator::graphUpdated);
    m_impl->graph = t_graph;

    if(m_impl->graph)
    {
        connect(m_impl->graph, &Graph::dirtyStateChanged, this, &GraphEvaluator::graphUpdated);
        if(t_graph->isDirty())
        {
            EvaluationContext context;
            t_graph->evaluate(&context);
            t_graph->markClean();
        }
    }

}

void GraphEvaluator::graphUpdated()
{
    if(!m_impl->timer.isActive() && m_impl->enabled)
        m_impl->timer.start(10);
}

void GraphEvaluator::timeout()
{
    if(!m_impl->enabled)
        return;

    EvaluationContext context;
    m_impl->graph->evaluate(&context);
    m_impl->graph->markClean();
    emit evaluationComplete();
}

} // namespace keira
