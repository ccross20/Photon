#ifndef CHANNELEFFECT_P_H
#define CHANNELEFFECT_P_H
#include "channeleffect.h"

namespace photon {

class ChannelEffect::Impl
{
public:
    QString name;
    QByteArray id;
    Channel *channel = nullptr;
    ChannelEffect *previousEffect = nullptr;
};

}

#endif // CHANNELEFFECT_P_H
