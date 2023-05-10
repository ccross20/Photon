#ifndef FIXTURECHANNEL_P_H
#define FIXTURECHANNEL_P_H
#include "fixturechannel.h"

namespace photon {

class FixtureChannel::Impl
{
public:
    void addCapability(const QJsonObject &);
    QVector<FixtureCapability*> capabilities;
    CapabilityType type = Capability_Unknown;
    QString name;
    QByteArrayList fineChannelsAliases;
    int channelNumber = -1;
    int globalChannelNumber;
    QVector<FixtureChannel*> fineChannels;
    Fixture *fixture = nullptr;
    FixtureChannel *facade;

};

}

#endif // FIXTURECHANNEL_P_H
