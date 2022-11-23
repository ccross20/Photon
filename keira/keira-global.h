#ifndef NODEGRAPHGLOBAL_H
#define NODEGRAPHGLOBAL_H

#include <QtCore/qglobal.h>
#include <QDebug>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

namespace keira
{

#if defined(KEIRA_LIBRARY)
#  define KEIRA_EXPORT Q_DECL_EXPORT
#else
#  define KEIRA_EXPORT Q_DECL_IMPORT
#endif

enum Connection
{
    NoConnection = 0,
    AllowSingleInput = 0x1,
    AllowMultipleInput = 0x2,
    AllowSingleOutput = 0x4,
    AllowMultipleOutput = 0x8
};

enum PortDirection
{
    Input,
    Output
};

using NodeCategoryId = QByteArray;
using NodeCategoryList = QVector<NodeCategoryId>;

struct EvaluationContext;
class Node;
class NodeItem;
class NodeLibrary;
class Graph;
class Port;
class Parameter;
class ParameterValue;
class Scene;

using NodeVector = QVector<Node*>;


}

#endif // NODEGRAPHGLOBAL_H
