#include "surfacenode.h"

#include "graph/parameter/dmxmatrixparameter.h"
#include "model/parameter/buttonparameter.h"
#include "surface/surface.h"
#include "surface/surfacecollection.h"
#include "photoncore.h"

namespace photon {

const QByteArray SurfaceNode::InputDMX = "dmxInput";
const QByteArray SurfaceNode::OutputDMX = "dmxOutput";

class SurfaceNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    keira::ButtonParameter *editParam;
    Surface *surface;
};

keira::NodeInformation SurfaceNode::info()
{
    keira::NodeInformation toReturn([](){return new SurfaceNode;});
    toReturn.name = "Surface Node";
    toReturn.nodeId = "photon.bus.surface";

    return toReturn;
}

SurfaceNode::SurfaceNode() : keira::Node("photon.bus.surface"),m_impl(new Impl)
{
    setName("Surface");
    m_impl->surface = new Surface("Test");
}

SurfaceNode::~SurfaceNode()
{
    delete m_impl;
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

void SurfaceNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();
    ProcessContext context{matrix};
    context.project = photonApp->project();


/*
    auto panel = photonApp->sequences()->activeSequencePanel();

    if(panel)
        panel->processPreview(context);
*/
    m_impl->surface->processChannels(context,0);

    m_impl->dmxOutParam->setValue(matrix);
}

void SurfaceNode::buttonClicked(const keira::Parameter *)
{
    photonApp->surfaces()->addSurface(m_impl->surface);
    photonApp->surfaces()->editSurface(m_impl->surface);
}

} // namespace photon
