#ifndef PHOTON_FIXTURECOLLECTION_H
#define PHOTON_FIXTURECOLLECTION_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureCollection : public QObject
{
    Q_OBJECT
public:
    explicit FixtureCollection(QObject *parent = nullptr);
    ~FixtureCollection();

    void addFixture(Fixture *t_fixture);
    void removeFixture(Fixture *t_fixture);
    void removeAllFixtures();
    int fixtureCount() const;
    Fixture *fixtureAtIndex(uint) const;

    QVector<Fixture*> fixturesWithName(const QString &);

    const QVector<Fixture*> &fixtures() const;

signals:

    void fixtureWillBeAdded(photon::Fixture *, int);
    void fixtureWasAdded(photon::Fixture *);
    void fixtureWillBeRemoved(photon::Fixture *, int);
    void fixtureWasRemoved(photon::Fixture *);
private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_FIXTURECOLLECTION_H
