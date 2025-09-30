#include "writedmxchannelnode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {


const QByteArray WriteDMXChannelNode::OutputDMX = "dmxout";
const QByteArray WriteDMXChannelNode::InputDMX = "dmxin";
const QByteArray WriteDMXChannelNode::Fixture = "fixture";
const QByteArray WriteDMXChannelNode::ChannelNumber = "channelNum";
const QByteArray WriteDMXChannelNode::ChannelValue = "channelVal";

class WriteDMXChannelNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    keira::IntegerParameter *channelParam;
    keira::IntegerParameter *valueParam;
    FixtureParameter *fixtureParam;
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
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX,"DMX In", DMXMatrix(), keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_impl->dmxInParam);

    m_impl->fixtureParam = new FixtureParameter(Fixture,"Fixture","");
    addParameter(m_impl->fixtureParam);

    m_impl->channelParam = new keira::IntegerParameter(ChannelNumber,"Channel Num",0);
    addParameter(m_impl->channelParam);

    m_impl->valueParam = new keira::IntegerParameter(ChannelValue,"Channel Value",0);
    addParameter(m_impl->valueParam);


    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX,"DMX Out", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxOutParam);
}

void WriteDMXChannelNode::evaluate(keira::EvaluationContext *) const
{

    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();

    QByteArray fixtureId = m_impl->fixtureParam->value().toByteArray();
    fixtureId = "test";
    if(!fixtureId.isEmpty())
    {

        //auto fixture = FixtureCollection::fixtureById(fixtureId);
        auto fixture = photonApp->project()->fixtures()->fixtures().first();

        if(!fixture)
            qDebug() << "Could not find fixture: " << fixtureId;
        else
        {
            if(matrix.channels.size() >= fixture->universe())
                matrix.setValueIntFloor(fixture->universe()-1, fixture->dmxOffset() + m_impl->channelParam->value().toInt(), m_impl->valueParam->value().toInt());
            else
                qDebug() << "DMX matrix isn't large enough: " << matrix.channels.size();
        }
    }

    m_impl->dmxOutParam->setValue(matrix);
}

} // namespace photon
