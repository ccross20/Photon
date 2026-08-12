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
    QByteArray surfaceId;

    // Resolved lazily from surfaceId and held until the collection changes.
    // keira::Node isn't a QObject so we can't listen for that — instead we
    // compare the collection's revision, which is cheaper than a lookup. The
    // collection pointer is part of the key because a node can be asked for its
    // surface while a *different* project is still the current one (Project::load
    // runs before PhotonCore::setProject), and revisions aren't comparable across
    // collections.
    mutable Surface *cachedSurface = nullptr;
    mutable const SurfaceCollection *cachedCollection = nullptr;
    mutable quint32 cachedRevision = 0;
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
}

SurfaceNode::~SurfaceNode()
{
    // The Project owns the surface — never delete it here.
    delete m_impl;
}

QByteArray SurfaceNode::surfaceId() const
{
    return m_impl->surfaceId;
}

void SurfaceNode::setSurfaceId(const QByteArray &t_id)
{
    m_impl->surfaceId = t_id;
    m_impl->cachedCollection = nullptr;
    m_impl->cachedSurface = nullptr;
}

Surface *SurfaceNode::surface() const
{
    if(m_impl->surfaceId.isEmpty())
        return nullptr;

    auto *collection = photonApp->surfaces();
    if(!collection)
        return nullptr;

    if(m_impl->cachedCollection == collection && m_impl->cachedRevision == collection->revision())
        return m_impl->cachedSurface;

    m_impl->cachedSurface = collection->findSurfaceWithId(m_impl->surfaceId);
    m_impl->cachedCollection = collection;
    m_impl->cachedRevision = collection->revision();

    return m_impl->cachedSurface;
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
    Surface *surface = this->surface();
    context->surface = surface;

    // The surface is owned by the project and referenced by id, so it can be
    // absent — deleted from the project panel, or an id that never resolved.
    // Pass DMX through untouched rather than dropping the frame.
    if(!surface)
    {
        m_impl->dmxOutParam->setValue(m_impl->dmxInParam->value());
        return;
    }

    // Publish every gizmo's live outputs onto the value bus so GizmoValueNodes
    // downstream can read them by "<uniqueId>/<portId>" without type coupling.
    for(auto *gizmo : surface->gizmos())
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

    surface->processChannels(processContext,0);

    keira::SubGraphNode::evaluate(context);

    m_impl->dmxOutParam->setValue(context->dmxMatrix);

}

void SurfaceNode::buttonClicked(const keira::Parameter *)
{
    if(auto *surface = this->surface())
        photonApp->surfaces()->editSurface(surface);
}

void SurfaceNode::readFromJson(const QJsonObject &t_obj, keira::NodeLibrary *t_library)
{
    setSurfaceId(t_obj.value("surfaceId").toString().toLatin1());
    SubGraphNode::readFromJson(t_obj, t_library);
}

void SurfaceNode::writeToJson(QJsonObject &t_obj) const
{
    SubGraphNode::writeToJson(t_obj);

    t_obj.insert("surfaceId", QString::fromLatin1(m_impl->surfaceId));
}

} // namespace photon
