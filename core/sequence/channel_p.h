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
    Clip *clip = nullptr;
};
}

#endif // CHANNEL_P_H
