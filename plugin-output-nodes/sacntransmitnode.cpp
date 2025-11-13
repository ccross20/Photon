#include <QNetworkInterface>
#include "sacntransmitnode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/integerparameter.h"
#include "streamingacn.h"
#include "sacnsender.h"

namespace photon {


const QByteArray SACNTransmitNode::InputDMX = "dmx";
const QByteArray SACNTransmitNode::Network = "network";

class SACNTransmitNode::Impl
{
public:
    void initialize();
    DMXMatrixParameter *dmxParam;
    keira::OptionParameter *networkParam;
    keira::IntegerParameter *initParam;
    sACNManager::tSender sender;
    bool isInitialized = false;
};

void SACNTransmitNode::Impl::initialize()
{
    sender->setSlotCount(512);
    sender->setName("Photon");
    sender->setUniverse(1);

    auto interfaces = QNetworkInterface::allInterfaces();
    sender->setNetworkInterface(interfaces[networkParam->value().toInt()]);
    sender->startSending();
    isInitialized = true;
}

keira::NodeInformation SACNTransmitNode::info()
{
    keira::NodeInformation toReturn([](){return new SACNTransmitNode;});
    toReturn.name = "sACN Transmit";
    toReturn.nodeId = "photon.bus.sacn-transmit-node";

    return toReturn;
}


SACNTransmitNode::SACNTransmitNode() : keira::Node("photon.bus.sacn-transmit-node"),m_impl(new Impl)
{
    setName("sACN Transmit");
    m_impl->sender = sACNManager::getInstance()->getSender(1);
}

SACNTransmitNode::~SACNTransmitNode()
{
    delete m_impl;
}

void SACNTransmitNode::createParameters()
{
    m_impl->dmxParam = new DMXMatrixParameter(InputDMX,"DMX Data", DMXMatrix());
    addParameter(m_impl->dmxParam);

    QStringList interfaces;
    for(const auto &ni : QNetworkInterface::allInterfaces())
    {
        interfaces << ni.humanReadableName();
    }

    m_impl->networkParam = new keira::OptionParameter(Network,"Network",interfaces,0);
    addParameter(m_impl->networkParam);

    m_impl->initParam = new keira::IntegerParameter("init","Initialize", 0);
    addParameter(m_impl->initParam);
}

void SACNTransmitNode::evaluate(keira::EvaluationContext *) const
{
    if(m_impl->initParam->value().toInt() > 0 && !m_impl->isInitialized)
        m_impl->initialize();

    if(!m_impl->isInitialized)
        return;

    const DMXMatrix &matrix = m_impl->dmxParam->value().value<DMXMatrix>();

/*
    float val1 = matrix.value(0,43);
    float val2 = matrix.value(0,44);
    float val3 = matrix.value(0,45);

    if(val1 == 0)
        val1 = 1;
    if(val2 == 0)
        val2 = 1;
    if(val3 == 0)
        val3 = 1;

    float val4 = matrix.value(0,47);
    float val5 = matrix.value(0,48);
    float val6 = matrix.value(0,49);

    if(val4 == 0)
        val4 = 1;
    if(val5 == 0)
        val5 = 1;
    if(val6 == 0)
        val6 = 1;
*/

    //qDebug() << QString::number(val1/val2, 'g',3) << QString::number(val2/val3, 'g',3) << QString::number(val4/val5, 'g',3) << QString::number(val5/val6, 'g',3);

    m_impl->sender->setLevel(matrix.channels[0].data(), 512,0);


}

} // namespace photon
