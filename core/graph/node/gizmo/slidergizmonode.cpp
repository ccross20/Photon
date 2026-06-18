#include "slidergizmonode.h"
#include "surface/surfacegraph.h"
#include "surface/surface.h"
#include "surface/slidergizmo.h"
#include "routine/routineevaluationcontext.h"
#include "graph/bus/surfacenode.h"

namespace photon {

const QByteArray SliderGizmoNode::GizmoId = "gizmoId";
const QByteArray SliderGizmoNode::Value = "value";



keira::NodeInformation SliderGizmoNode::info()
{
    keira::NodeInformation toReturn([](){return new SliderGizmoNode;});
    toReturn.name = "Slider Gizmo";
    toReturn.nodeId = "photon.gizmo.slider-gizmo";
    toReturn.categories = {"Gizmo"};
    toReturn.graphs = QByteArrayList{"surface"};

    return toReturn;
}

SliderGizmoNode::SliderGizmoNode() : keira::Node("photon.gizmo.slider-gizmo") {
    setName("Slider Gizmo");
}

void SliderGizmoNode::addedToGraph(keira::Graph *t_graph)
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

void SliderGizmoNode::createParameters()
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
                    auto gizmos = surfaceNode->surface()->gizmosByType(SliderGizmo::GizmoId);
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
                auto gizmos = surfaceNode->surface()->gizmosByType(SliderGizmo::GizmoId);
                if(!gizmos.isEmpty())
                    m_optionParam->setValue(gizmos.first()->uniqueId());
            }
        }
    }


    addParameter(m_optionParam);
    m_valueParam = new keira::DecimalParameter(Value,"Output", false, keira::AllowMultipleOutput);
    addParameter(m_valueParam);


}

void SliderGizmoNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto context = static_cast<RoutineEvaluationContext*>(t_context);

    QByteArray gizmoId = m_optionParam->value().toByteArray();

    if(context->surface)
    {
        SliderGizmo *gizmo = dynamic_cast<SliderGizmo*>(context->surface->findGizmoWithUniqueId(gizmoId));
        if(gizmo)
        {
            m_valueParam->setValue(gizmo->value());
        }

    }


}

} // namespace photon
