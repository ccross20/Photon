#ifndef PHOTON_FIXTUREMASK_H
#define PHOTON_FIXTUREMASK_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureMask : public QObject
{
    Q_OBJECT
public:
    explicit FixtureMask(QObject *parent = nullptr);
    ~FixtureMask();


    void addFixture(Fixture *t_fixture);
    void removeFixture(Fixture *t_fixture);

    const QSet<Fixture*> &fixtures() const;

signals:

    void FixtureAdded(photon::Fixture*);
    void FixtureRemoved(photon::Fixture*);

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_FIXTUREMASK_H
