#ifndef GRAPH_P_H
#define GRAPH_P_H

#include "graph.h"
#include <QMutex>
#include <functional>

namespace keira
{
class Graph::Impl
{
public:
    QVector<Node*> nodes;

    Node *parentNode = nullptr;
    QByteArray uniqueId;
    QByteArray graphTypeId;
    QString name;
    int dirty = DirtyModes::Clean;

    // Thread-safe command queue — UI thread pushes, eval thread drains at frame start.
    QMutex commandMutex;
    QVector<std::function<void()>> pendingCommands;

    // Guards `nodes` itself. Draining is meant to happen only from the eval thread
    // (paired with evaluate() each tick), but some editor code paths force an
    // out-of-band drain from the GUI thread to get an immediate result (see
    // SubGraphNode::exposeInputForType) - without this, that races the eval
    // thread's own drainCommandQueue()/evaluate()/prepForEvaluation()/markClean(),
    // which iterate or mutate this same vector unsynchronized.
    mutable QMutex nodesMutex;
};

}


#endif // GRAPH_P_H
