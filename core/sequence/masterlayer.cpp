#include "masterlayer.h"
#include "channel.h"

namespace photon {

class MasterLayer::Impl
{
public:
    Channel *channel = nullptr;
};

MasterLayer::MasterLayer(QObject *parent) : Layer("", "MasterLayer",parent),m_impl(new Impl)
{

}

MasterLayer::MasterLayer(const QString &name, QObject *parent) : Layer(name, "MasterLayer", parent),m_impl(new Impl)
{
    ChannelInfo info;
    info.name = "Master Channel";
    info.type = ChannelInfo::ChannelTypeNumber;
    m_impl->channel = new Channel(info, 0, 9000, this);
}

MasterLayer::~MasterLayer()
{
    delete m_impl;
}

Channel *MasterLayer::channel() const
{
    return m_impl->channel;
}

double MasterLayer::process(double time) const
{
    return m_impl->channel->processDouble(time);
}

void MasterLayer::processChannels(ProcessContext &t_context)
{

}

void MasterLayer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Layer::readFromJson(t_json, t_context);

    QJsonObject channelJson = t_json.value("channel").toObject();
    if(!m_impl->channel)
        m_impl->channel = new Channel(ChannelInfo(), 0, 9000, this);
    m_impl->channel->readFromJson(channelJson, t_context);
}

void MasterLayer::writeToJson(QJsonObject &t_json) const
{
    Layer::writeToJson(t_json);


    QJsonObject channelObj;
    m_impl->channel->writeToJson(channelObj);
    t_json.insert("channel",channelObj);
}

} // namespace photon
