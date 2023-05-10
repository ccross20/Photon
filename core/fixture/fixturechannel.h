#ifndef PHOTON_FIXTURECHANNEL_H
#define PHOTON_FIXTURECHANNEL_H
#include "photon-global.h"
#include "capability/fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureChannel
{
public:
    FixtureChannel(const QString &name, int channelNumber = -1);
    ~FixtureChannel();

    QString name() const;
    int channelNumber() const;
    int universalChannelNumber() const;
    int universe() const;
    void setChannelNumber(int channelNum);
    bool isValid() const;
    const QVector<FixtureChannel*> &fineChannels() const;

    CapabilityType capabilityType() const;

    Fixture *fixture() const;
    void setFixture(Fixture *);

    const QVector<FixtureCapability*> &capabilities() const;

    void readFromOpenFixtureJson(const QJsonObject &);

private:
    friend class Fixture;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECHANNEL_H
