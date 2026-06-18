#include "writedmxchannelnode.h"
#include "model/parameter/integerparameter.h"
#include "fixture/fixture.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

const QByteArray WriteDMXChannelNode::ChannelNumber = "channelNum";
const QByteArray WriteDMXChannelNode::ChannelValue = "channelVal";

class WriteDMXChannelNode::Impl
{
public:
    keira::IntegerParameter *channelParam;
    keira::IntegerParameter *valueParam;
};

keira::NodeInformation WriteDMXChannelNode::info()
{
    keira::NodeInformation toReturn([](){return new WriteDMXChannelNode;});
    toReturn.name = "Write DMX Channel";
    toReturn.nodeId = "photon.bus.write-dmx-channel";

    return toReturn;
}


WriteDMXChannelNode::WriteDMXChannelNode() : keira::Node("photon.bus.write-dmx-channel"),m_impl(new Impl)
{
    setName("Write DMX Channel");
}

WriteDMXChannelNode::~WriteDMXChannelNode()
{
    delete m_impl;
}

void WriteDMXChannelNode::createParameters()
{
    m_impl->channelParam = new keira::IntegerParameter(ChannelNumber,"Channel Num",1);
    addParameter(m_impl->channelParam);

    m_impl->valueParam = new keira::IntegerParameter(ChannelValue,"Channel Value",256);
    addParameter(m_impl->valueParam);

}

void WriteDMXChannelNode::evaluate(keira::EvaluationContext *t_context) const
{


    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        context->dmxMatrix.setValue(context->fixture->universe() - 1, context->fixture->dmxOffset() + (m_impl->channelParam->value().toInt()-1), m_impl->valueParam->value().toInt()-1);
    }

}

} // namespace photon
