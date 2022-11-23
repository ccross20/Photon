#ifndef GRAPH_P_H
#define GRAPH_P_H

#include "graph.h"

namespace keira
{
class Graph::Impl
{
public:
    QVector<Node*> nodes;

    QByteArray uniqueId;
    bool isDirty = true;
};

}


#endif // GRAPH_P_H
