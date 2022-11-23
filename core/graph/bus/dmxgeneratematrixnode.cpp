#include "dmxgeneratematrixnode.h"
#include "graph/parameter/dmxmatrixparameter.h"

namespace photon {


const QByteArray DMXGenerateMatrixNode::OutputDMX = "dmx";

class DMXGenerateMatrixNode::Impl
{
public:
    DMXMatrixParameter *dmxParam;
};

keira::NodeInformation DMXGenerateMatrixNode::info()
{
    keira::NodeInformation toReturn([](){return new DMXGenerateMatrixNode;});
    toReturn.name = "DMX Generator";
    toReturn.nodeId = "photon.bus.dmx-generator";

    return toReturn;
}

DMXGenerateMatrixNode::DMXGenerateMatrixNode() : keira::Node("photon.bus.dmx-generator"),m_impl(new Impl)
{
    setName("DMX Generator");
}

DMXGenerateMatrixNode::~DMXGenerateMatrixNode()
{
    delete m_impl;
}

void DMXGenerateMatrixNode::createParameters()
{
    m_impl->dmxParam = new DMXMatrixParameter(OutputDMX,"DMX Data", DMXMatrix());
    addParameter(m_impl->dmxParam);
}

void DMXGenerateMatrixNode::evaluate(keira::EvaluationContext *) const
{

}

} // namespace photon

