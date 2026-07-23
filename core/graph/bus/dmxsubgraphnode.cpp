#include "dmxsubgraphnode.h"

#include "graph/parameter/dmxmatrixparameter.h"
#include "routine/routineevaluationcontext.h"
#include "graph/bus/busgraph.h"

namespace photon {

const QByteArray DMXSubGraphNode::InputDMX = "dmxInput";
const QByteArray DMXSubGraphNode::OutputDMX = "dmxOutput";

class DMXSubGraphNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
};

keira::NodeInformation DMXSubGraphNode::info()
{
    keira::NodeInformation toReturn([](){return new DMXSubGraphNode;});
    toReturn.name = "DMX SubGraph";
    toReturn.nodeId = "photon.bus.dmx-subgraph";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{BusGraph::BusGraphId};

    return toReturn;
}

DMXSubGraphNode::DMXSubGraphNode() : keira::SubGraphNode("photon.bus.dmx-subgraph"), m_impl(new Impl)
{
    setName("DMX SubGraph");
    graph()->setGraphTypeId("dmx-subgraph");
}

DMXSubGraphNode::~DMXSubGraphNode()
{
    delete m_impl;
}

void DMXSubGraphNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX, "DMX Input", DMXMatrix());
    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX, "DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxInParam);
    addParameter(m_impl->dmxOutParam);
}

void DMXSubGraphNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto context = static_cast<RoutineEvaluationContext*>(t_context);

    keira::SubGraphNode::evaluate(context);

    m_impl->dmxOutParam->setValue(context->dmxMatrix);
}

} // namespace photon
