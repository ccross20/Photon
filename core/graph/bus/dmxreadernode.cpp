#include "dmxreadernode.h"
#include "graph/parameter/dmxmatrixparameter.h"

namespace photon {


const QByteArray DMXReaderNode::OutputDMX = "dmx";

class DMXReaderNode::Impl
{
public:
    DMXMatrixParameter *dmxParam;
};

keira::NodeInformation DMXReaderNode::info()
{
    keira::NodeInformation toReturn([](){return new DMXReaderNode;});
    toReturn.name = "DMX Reader";
    toReturn.nodeId = "photon.bus.dmx-reader";

    return toReturn;
}


DMXReaderNode::DMXReaderNode() : keira::Node("photon.bus.dmx-reader"),m_impl(new Impl)
{
    setName("DMX Reader");
}

DMXReaderNode::~DMXReaderNode()
{
    delete m_impl;
}

void DMXReaderNode::createParameters()
{
    m_impl->dmxParam = new DMXMatrixParameter(OutputDMX,"DMX Data", DMXMatrix());
    addParameter(m_impl->dmxParam);
}

void DMXReaderNode::evaluate(keira::EvaluationContext *) const
{
    /*
    const DMXMatrix &matrix = m_impl->dmxParam->value().value<DMXMatrix>();

    if(matrix.channels.size() > 0)
        qDebug() << matrix.channels[0][0];
    else
        qDebug() << "DMX is empty";
        */
}

} // namespace photon
