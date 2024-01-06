#include "sequencenode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "model/parameter/buttonparameter.h"
#include "sequence/sequence.h"
#include "gui/panel/sequencepanel.h"
#include "sequence/sequencecollection.h"
#include "photoncore.h"

namespace photon {

const QByteArray SequenceNode::InputDMX = "dmxInput";
const QByteArray SequenceNode::OutputDMX = "dmxOutput";

class SequenceNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    keira::ButtonParameter *editParam;
    Sequence sequence;
};

keira::NodeInformation SequenceNode::info()
{
    keira::NodeInformation toReturn([](){return new SequenceNode;});
    toReturn.name = "Sequence Node";
    toReturn.nodeId = "photon.bus.sequence";

    return toReturn;
}

SequenceNode::SequenceNode() : keira::Node("photon.bus.sequence"),m_impl(new Impl)
{
    setName("Sequence");
}

SequenceNode::~SequenceNode()
{
    delete m_impl;
}

void SequenceNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX,"DMX Input", DMXMatrix());
    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX,"DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    m_impl->editParam = new keira::ButtonParameter("Edit","Edit");
    m_impl->editParam->setLayoutOptions(keira::Parameter::LayoutNoLabel);
    addParameter(m_impl->dmxInParam);
    addParameter(m_impl->editParam);
    addParameter(m_impl->dmxOutParam);
}

void SequenceNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();
    ProcessContext context{matrix};
    context.project = photonApp->project();



    auto panel = photonApp->sequences()->activeSequencePanel();

    if(panel)
        panel->processPreview(context);
    //m_impl->sequence.processChannels(context,0);

    m_impl->dmxOutParam->setValue(matrix);
}

void SequenceNode::buttonClicked(const keira::Parameter *)
{

}

} // namespace photon
