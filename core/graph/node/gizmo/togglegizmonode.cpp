#include "togglegizmonode.h"
#include "surface/surfacegraph.h"
#include "surface/surface.h"
#include "surface/togglegizmo.h"
#include "routine/routineevaluationcontext.h"
#include "graph/bus/surfacenode.h"

namespace photon {


const QByteArray ToggleGizmoNode::GizmoId = "gizmoId";
const QByteArray ToggleGizmoNode::Value = "value";



keira::NodeInformation ToggleGizmoNode::info()
{
    keira::NodeInformation toReturn([](){return new ToggleGizmoNode;});
    toReturn.name = "Toggle Gizmo";
    toReturn.nodeId = "photon.gizmo.toggle-gizmo";
    toReturn.categories = {"Gizmo"};
    toReturn.graphs = QByteArrayList{"surface"};

    return toReturn;
}

ToggleGizmoNode::ToggleGizmoNode() : keira::Node("photon.gizmo.toggle-gizmo") {
    setName("Toggle Gizmo");
}

void ToggleGizmoNode::addedToGraph(keira::Graph *t_graph)
{
    if(m_optionParam->value().toInt() == 0)
    {
        auto options = m_optionParam->optionLambda()();

        if(!options.isEmpty())
        {
            m_optionParam->setValue(options.first().second);
        }
    }
}

void ToggleGizmoNode::createParameters()
{
    m_optionParam = new keira::StringOptionParameter(GizmoId,"Gizmo", {},0, keira::AllowMultipleOutput | keira::AllowSingleInput);
    m_optionParam->setOptionLambda([this](){
        QVector<std::pair<QString,QString>> results;

        auto nodes = nodeHierarchy();
        for(auto node : nodes)
        {
            if(node->id() == "photon.bus.surface")
            {
                auto surfaceNode = static_cast<SurfaceNode*>(node);

                if(surfaceNode->surface())
                {
                    auto gizmos = surfaceNode->surface()->gizmosByType(ToggleGizmo::GizmoId);
                    for(auto gizmo : gizmos){
                        results.append(std::pair<QString,QString>(gizmo->id(),gizmo->uniqueId()));
                    }
                }
            }
            break;
        }


        return results;
    });


    auto nodes = nodeHierarchy();
    for(auto node : nodes)
    {
        if(node->id() == "photon.bus.surface")
        {
            auto surfaceNode = static_cast<SurfaceNode*>(node);
            if(surfaceNode->surface())
            {
                auto gizmos = surfaceNode->surface()->gizmosByType(ToggleGizmo::GizmoId);
                if(!gizmos.isEmpty())
                    m_optionParam->setValue(gizmos.first()->uniqueId());
            }
        }
    }


    addParameter(m_optionParam);
    m_valueParam = new keira::BooleanParameter(Value,"Output", false, keira::AllowMultipleOutput);
    addParameter(m_valueParam);


}

void ToggleGizmoNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto context = static_cast<RoutineEvaluationContext*>(t_context);

    QByteArray gizmoId = m_optionParam->value().toByteArray();

    if(context->surface)
    {
        ToggleGizmo *gizmo = dynamic_cast<ToggleGizmo*>(context->surface->findGizmoWithUniqueId(gizmoId));
        if(gizmo)
        {
            m_valueParam->setValue(gizmo->isPressed());
        }

    }


}

} // namespace photon
