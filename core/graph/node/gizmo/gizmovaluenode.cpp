#include "gizmovaluenode.h"
#include "surface/surface.h"
#include "surface/surfacegizmo.h"
#include "graph/bus/surfacenode.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

const QByteArray GizmoValueNode::Source = "source";
const QByteArray GizmoValueNode::Value = "value";

keira::NodeInformation GizmoValueNode::info()
{
    keira::NodeInformation toReturn([](){return new GizmoValueNode;});
    toReturn.name = "Gizmo Value";
    toReturn.nodeId = "photon.gizmo.gizmo-value";
    toReturn.categories = {"Gizmo"};
    toReturn.graphs = QByteArrayList{"surface"};
    return toReturn;
}

GizmoValueNode::GizmoValueNode() : keira::Node("photon.gizmo.gizmo-value")
{
    setName("Gizmo Value");
}

// Walk up the node hierarchy to the owning surface, if any.
static Surface *findSurface(const keira::Node *t_node)
{
    for(auto *node : t_node->nodeHierarchy())
    {
        if(node->id() == "photon.bus.surface")
            return static_cast<SurfaceNode*>(node)->surface();
    }
    return nullptr;
}

// A gizmo's display label for the Source dropdown: its user-assigned Name if
// one was given, otherwise its type plus a short suffix from its GUID — more
// recognizable than the raw GUID while still disambiguating unnamed controls
// of the same type.
static QString gizmoLabelFor(const SurfaceGizmo *t_gizmo)
{
    if(!t_gizmo->id().isEmpty())
        return QString(t_gizmo->id());
    const QString uniqueId = QString(t_gizmo->uniqueId());
    return t_gizmo->gizmoTypeString() + " " + uniqueId.right(4);
}

// Every gizmo->output pair on the surface, as (label, "<uniqueId>/<portId>").
static QVector<std::pair<QString,QString>> sourceOptions(const keira::Node *t_node)
{
    QVector<std::pair<QString,QString>> results;
    Surface *surface = findSurface(t_node);
    if(!surface)
        return results;

    for(auto *gizmo : surface->gizmos())
    {
        const QString gizmoLabel = gizmoLabelFor(gizmo);
        const auto outputs = gizmo->outputs();
        for(const auto &output : outputs)
        {
            // Only qualify with the port name when the gizmo has more than one.
            const QString label = outputs.size() > 1 ? gizmoLabel + " : " + output.name : gizmoLabel;
            results.append({label, QString(gizmo->uniqueId() + "/" + output.id)});
        }
    }
    return results;
}

void GizmoValueNode::createParameters()
{
    m_sourceParam = new keira::StringOptionParameter(Source, "Source", {}, 0,
                                                     keira::AllowMultipleOutput | keira::AllowSingleInput);
    m_sourceParam->setOptionLambda([this](){ return sourceOptions(this); });
    addParameter(m_sourceParam);

    m_valueParam = new keira::AnyParameter(Value, "Value", keira::AllowMultipleOutput);
    addParameter(m_valueParam);
}

void GizmoValueNode::addedToGraph(keira::Graph *)
{
    // Default to the first available source once we can see the surface, so a
    // freshly dropped node has a valid selection. The param's default is the
    // int index 0 (not a real key), so we can't test for "empty" — instead
    // check whether the current value matches a known option.
    const auto options = sourceOptions(this);
    if(options.isEmpty())
        return;

    const QString current = m_sourceParam->value().toString();
    bool valid = false;
    for(const auto &option : options)
    {
        if(option.second == current)
        {
            valid = true;
            break;
        }
    }

    if(!valid)
        m_sourceParam->setValue(options.first().second);
}

void GizmoValueNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto context = static_cast<RoutineEvaluationContext*>(t_context);

    const QByteArray key = m_sourceParam->value().toByteArray();
    const auto it = context->gizmoValues.constFind(key);
    if(it != context->gizmoValues.constEnd())
        m_valueParam->setValue(it.value());
}

} // namespace photon
