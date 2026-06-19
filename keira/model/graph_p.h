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
};

}


#endif // GRAPH_P_H
