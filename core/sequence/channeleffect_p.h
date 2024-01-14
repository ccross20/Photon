#ifndef CHANNELEFFECT_P_H
#define CHANNELEFFECT_P_H
#include "channeleffect.h"

namespace photon {

class ChannelEffect::Impl
{
public:
    void addToChannel(Channel*);
    QString name;
    QByteArray id;
    QByteArray uniqueId;
    Channel *channel = nullptr;
    ChannelEffect *previousEffect = nullptr;
    ChannelEffect *facade;
    ChannelEffectViewState viewState;
};

}

#endif // CHANNELEFFECT_P_H
