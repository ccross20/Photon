#ifndef PHOTON_PARAMETERCONTAINER_H
#define PHOTON_PARAMETERCONTAINER_H
#include "photon-global.h"
#include "sequence/channel.h"

namespace photon {

class PHOTONCORE_EXPORT ChannelParameterContainer
{
public:
    ChannelParameterContainer();
    virtual ~ChannelParameterContainer();


    void addChannelParameter(ChannelParameter *);
    void removeChannelParameter(ChannelParameter *);
    ChannelParameter *channelParameterAtIndex(int index) const;
    int channelParameterCount() const;
    const QVector<ChannelParameter*> channelParameters() const;

    QHash<QByteArray,QVariant> valuesFromChannels(const QVector<Channel*> &, double) const;
    QVector<ChannelInfo> createChannelsFromParameter(ChannelParameter *, ChannelInfo::ChannelType type = ChannelInfo::ChannelTypeNumber);

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PARAMETERCONTAINER_H
