#ifndef PHOTON_FIXTURECHANNEL_H
#define PHOTON_FIXTURECHANNEL_H
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureChannel
{
public:
    FixtureChannel(const QString &name, int channelNumber = -1);
    ~FixtureChannel();

    QString name() const;
    int channelNumber() const;
    int universalChannelNumber() const;
    void setChannelNumber(uchar channelNum);

    Fixture *fixture() const;
    void setFixture(Fixture *);

    const QVector<FixtureCapability*> &capabilities() const;

    void readFromOpenFixtureJson(const QJsonObject &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECHANNEL_H
