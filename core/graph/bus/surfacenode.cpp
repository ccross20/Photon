#include "surfacenode.h"

#include "graph/parameter/dmxmatrixparameter.h"
#include "model/parameter/buttonparameter.h"
#include "surface/surface.h"
#include "surface/surfacegizmo.h"
#include "surface/surfacecollection.h"
#include "photoncore.h"
#include "routine/routineevaluationcontext.h"
#include "surface/surfacegraph.h"
#include "graph/bus/busgraph.h"

namespace photon {

const QByteArray SurfaceNode::InputDMX = "dmxInput";
const QByteArray SurfaceNode::OutputDMX = "dmxOutput";

class SurfaceNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    keira::ButtonParameter *editParam;
    Surface *surface = nullptr;
    QByteArray surfaceId;
};

keira::NodeInformation SurfaceNode::info()
{
    keira::NodeInformation toReturn([](){return new SurfaceNode;});
    toReturn.name = "Surface Node";
    toReturn.nodeId = "photon.bus.surface";
    toReturn.graphs = QByteArrayList{BusGraph::BusGraphId};

    return toReturn;
}

SurfaceNode::SurfaceNode() : keira::SubGraphNode("photon.bus.surface"),m_impl(new Impl)
{
    setName("Surface");
    graph()->setGraphTypeId("surface");
    m_impl->surface = new Surface();
}

SurfaceNode::~SurfaceNode()
{
    delete m_impl->surface;
    delete m_impl;
}

Surface *SurfaceNode::surface() const
{
    return m_impl->surface;
}

void SurfaceNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX,"DMX Input", DMXMatrix());
    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX,"DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    m_impl->editParam = new keira::ButtonParameter("View","View");
    m_impl->editParam->setLayoutOptions(keira::Parameter::LayoutNoLabel);
    addParameter(m_impl->dmxInParam);
    addParameter(m_impl->editParam);
    addParameter(m_impl->dmxOutParam);
}

void SurfaceNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto context = static_cast<RoutineEvaluationContext*>(t_context);
    context->surface = m_impl->surface;

    // Publish every gizmo's live outputs onto the value bus so GizmoValueNodes
    // downstream can read them by "<uniqueId>/<portId>" without type coupling.
    for(auto *gizmo : m_impl->surface->gizmos())
    {
        const QByteArray prefix = gizmo->uniqueId() + "/";
        for(const auto &output : gizmo->outputs())
            context->gizmoValues.insert(prefix + output.id, gizmo->outputValue(output.id));
    }

    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();
    ProcessContext processContext{matrix};
    processContext.project = photonApp->project();
    processContext.globalTime = context->globalTime;
    processContext.relativeTime = context->relativeTime;

/*
    auto panel = photonApp->sequences()->activeSequencePanel();

    if(panel)
        panel->processPreview(context);
*/
    m_impl->surface->processChannels(processContext,0);

    keira::SubGraphNode::evaluate(context);

    m_impl->dmxOutParam->setValue(context->dmxMatrix);

}

void SurfaceNode::buttonClicked(const keira::Parameter *)
{

    photonApp->surfaces()->addSurface(m_impl->surface);
    photonApp->surfaces()->editSurface(m_impl->surface);
}

void SurfaceNode::readFromJson(const QJsonObject &t_obj, keira::NodeLibrary *t_library)
{
    LoadContext context;
    m_impl->surface->readFromJson(t_obj.value("surface").toObject(), context);
    SubGraphNode::readFromJson(t_obj, t_library);



}

void SurfaceNode::writeToJson(QJsonObject &t_obj) const
{
    SubGraphNode::writeToJson(t_obj);

    QJsonObject surfaceObj;
    m_impl->surface->writeToJson(surfaceObj);
    t_obj.insert("surface", surfaceObj);
}

} // namespace photon
