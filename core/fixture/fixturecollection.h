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

    QVector<Fixture*> fixturesWithName(const QString &);
    static Fixture* fixtureById(const QByteArray &);
    static Fixture* fixtureById(Project *, const QByteArray &);
    Fixture *fixtureWithId(const QByteArray &) const;

    const QVector<Fixture*> fixtures() const;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_FIXTURECOLLECTION_H
