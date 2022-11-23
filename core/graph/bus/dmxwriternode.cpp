#include "dmxwriternode.h"
#include "graph/parameter/dmxmatrixparameter.h"

namespace photon {


const QByteArray DMXWriterNode::InputDMX = "dmx";

class DMXWriterNode::Impl
{
public:
    DMXMatrixParameter *dmxParam;
};

keira::NodeInformation DMXWriterNode::info()
{
    keira::NodeInformation toReturn([](){return new DMXWriterNode;});
    toReturn.name = "DMX Writer";
    toReturn.nodeId = "photon.bus.dmx-writer";

    return toReturn;
}


DMXWriterNode::DMXWriterNode() : keira::Node("photon.bus.dmx-writer"),m_impl(new Impl)
{
    setName("DMX Writer");
}

DMXWriterNode::~DMXWriterNode()
{
    delete m_impl;
}

void DMXWriterNode::createParameters()
{
    m_impl->dmxParam = new DMXMatrixParameter(InputDMX,"DMX Data", DMXMatrix());
    addParameter(m_impl->dmxParam);
}

void DMXWriterNode::evaluate(keira::EvaluationContext *) const
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
