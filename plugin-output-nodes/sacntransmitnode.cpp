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
    QVector<sACNManager::tSender> senders;
    bool isInitialized = false;
};

void SACNTransmitNode::Impl::initialize()
{
    auto interfaces = QNetworkInterface::allInterfaces();
    int counter = 1;
    for(auto it = senders.begin(); it!= senders.end(); ++it)
    {

        qDebug() << "Create universe" << counter;
        auto sender = *it;
        sender->setSlotCount(512);
        sender->setName("Photon" + QString::number(counter));
        sender->setUniverse(counter++);

        //if(sender->isSending())
        //sender->stopSending();


        sender->setNetworkInterface(interfaces[networkParam->value().toInt()]);
        sender->startSending();
    }



    isInitialized = true;
}

keira::NodeInformation SACNTransmitNode::info()
{
    keira::NodeInformation toReturn([](){return new SACNTransmitNode;});
    toReturn.name = "sACN Transmit";
    toReturn.nodeId = "photon.bus.sacn-transmit-node";
    toReturn.graphs = QByteArrayList{"bus"};

    return toReturn;
}


SACNTransmitNode::SACNTransmitNode() : keira::Node("photon.bus.sacn-transmit-node"),m_impl(new Impl)
{
    setName("sACN Transmit");

    for(int i = 1; i < 3; ++i)
    {
        m_impl->senders.append(sACNManager::getInstance()->getSender(i));
    }
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
    else if(m_impl->initParam->value().toInt() <= 0)
    {
        m_impl->isInitialized = false;
        for(auto it = m_impl->senders.begin(); it!= m_impl->senders.end(); ++it)
        {
            (*it)->stopSending();
        }
    }

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

    int i = 0;
    for(auto it = m_impl->senders.begin(); it!= m_impl->senders.end(); ++it)
    {
        if(i < matrix.channels.size())
            (*it)->setLevel(matrix.channels[i++].data(), 512,0);
    }

    //qDebug() << matrix.channels.size();

}

} // namespace photon
