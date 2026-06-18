#ifndef GRAPH_P_H
#define GRAPH_P_H

#include "graph.h"

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
};

}


#endif // GRAPH_P_H
