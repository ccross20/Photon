#ifndef NODEGRAPHGLOBAL_H
#define NODEGRAPHGLOBAL_H

#include <QtCore/qglobal.h>
#include <QDebug>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

class QMimeData;

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

enum DirtyModes
{
    Clean = 0,
    Dirty_Eval = 0x1,
    Dirty_Priority = 0x2,
    Dirty_Parameter = 0x4,
    Dirty_Structure = 0x8
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
class NodeEditor;
class NodeItem;
class NodeLibrary;
class Graph;
class Port;
class Parameter;
class ParameterValue;
class Scene;

using NodeVector = QVector<Node*>;

// One node to create in response to an external drag-and-drop landing on a
// Scene - e.g. an item dragged in from the host application's own asset
// browser, which keira has no knowledge of. The host supplies an
// ExternalDropInterpreter (see Scene::setExternalDropInterpreter) that
// decodes its own QMimeData into a list of these; keira just creates the
// named node (by NodeLibrary id) and, if paramName isn't empty, sets that
// one parameter to paramValue.
struct ExternalDropNodeSpec
{
    QByteArray nodeId;
    QByteArray paramName;
    QVariant paramValue;
};
using ExternalDropInterpreter = std::function<QVector<ExternalDropNodeSpec>(const QMimeData*)>;

}

#endif // NODEGRAPHGLOBAL_H
