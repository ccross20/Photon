#ifndef NODE_P_H
#define NODE_P_H
#include "node.h"
#include <QPointF>

namespace keira
{
class Node::Impl
{
public:

    QVector<Parameter*> parameters;
    QSet<Node*> dependents;
    QVariant emptyValue;
    QString name;
    QPointF position;
    QByteArray id;
    QByteArray uniqueId;
    Graph *graph;
    bool isDirty = true;
    bool visited = false;
};

}

#endif // NODE_P_H
