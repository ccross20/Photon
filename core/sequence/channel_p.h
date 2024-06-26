#ifndef CHANNEL_P_H
#define CHANNEL_P_H
#include "channel.h"

namespace photon
{
class Channel::Impl
{
public:
    ChannelInfo info;
    QVector<ChannelEffect*> effects;
    QVector<Channel*> subChannels;
    QByteArray uniqueId;
    QString parentName;
    double startTime = 0;
    double duration = 0;
};
}

#endif // CHANNEL_P_H
