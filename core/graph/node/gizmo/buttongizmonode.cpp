#include "buttongizmonode.h"
#include "surface/surfacegraph.h"
#include "surface/surface.h"
#include "surface/buttongizmo.h"
#include "routine/routineevaluationcontext.h"
#include "graph/bus/surfacenode.h"

namespace photon {


const QByteArray ButtonGizmoNode::GizmoId = "gizmoId";
const QByteArray ButtonGizmoNode::Value = "value";



keira::NodeInformation ButtonGizmoNode::info()
{
    keira::NodeInformation toReturn([](){return new ButtonGizmoNode;});
    toReturn.name = "Button Gizmo";
    toReturn.nodeId = "photon.gizmo.button-gizmo";
    toReturn.categories = {"Gizmo"};
    toReturn.graphs = QByteArrayList{"surface"};

    return toReturn;
}

ButtonGizmoNode::ButtonGizmoNode() : keira::Node("photon.gizmo.button-gizmo") {
    setName("Button Gizmo");
}

void ButtonGizmoNode::addedToGraph(keira::Graph *t_graph)
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

void ButtonGizmoNode::createParameters()
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
                    auto gizmos = surfaceNode->surface()->gizmosByType(ButtonGizmo::GizmoId);
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
                auto gizmos = surfaceNode->surface()->gizmosByType(ButtonGizmo::GizmoId);
                if(!gizmos.isEmpty())
                    m_optionParam->setValue(gizmos.first()->uniqueId());
            }
        }
    }


    addParameter(m_optionParam);
    m_valueParam = new keira::BooleanParameter(Value,"Output", false, keira::AllowMultipleOutput);
    addParameter(m_valueParam);


}

void ButtonGizmoNode::evaluate(keira::EvaluationContext *t_context) const
{

    auto context = static_cast<RoutineEvaluationContext*>(t_context);

    QByteArray gizmoId = m_optionParam->value().toByteArray();

    if(context->surface)
    {
        ButtonGizmo *gizmo = dynamic_cast<ButtonGizmo*>(context->surface->findGizmoWithUniqueId(gizmoId));
        if(gizmo)
        {
            m_valueParam->setValue(gizmo->isPressed());
        }

    }


}

} // namespace photon
