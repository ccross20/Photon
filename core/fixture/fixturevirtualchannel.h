#ifndef PHOTON_FIXTUREVIRTUALCHANNEL_H
#define PHOTON_FIXTUREVIRTUALCHANNEL_H

#include "photon-global.h"
#include "capability/fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureVirtualChannel
{
public:
    FixtureVirtualChannel(const QVector<FixtureChannel*> &, const QString &name = "");
    ~FixtureVirtualChannel();

    const QVector<FixtureChannel*> &channels() const;
    bool isValid() const;
    QString name() const;

    CapabilityType capabilityType() const;
    const QVector<FixtureCapability*> &capabilities() const;

    Fixture *fixture() const;
    void setFixture(Fixture *);

private:
    friend class Fixture;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREVIRTUALCHANNEL_H
