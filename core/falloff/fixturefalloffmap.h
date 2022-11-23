#ifndef PHOTON_FIXTUREFALLOFFMAP_H
#define PHOTON_FIXTUREFALLOFFMAP_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureFalloffMap : public QObject
{
    Q_OBJECT
public:
    explicit FixtureFalloffMap(QObject *parent = nullptr);
    virtual ~FixtureFalloffMap();

    void addFixture(Fixture *fixture, double delay);
    virtual double falloff(Fixture *);

signals:

    void Updated();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREFALLOFFMAP_H
