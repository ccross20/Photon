#ifndef PHOTON_FIXTUREGROUP_H
#define PHOTON_FIXTUREGROUP_H

#include <QObject>
#include <QStringList>
#include "photon-global.h"
#include "fixture/fixturequery.h"

class QJsonObject;

namespace photon {

// A named, saved fixture selection (a FixtureQuery with a name). Referenced by name
// from shows so a selection can be defined once and reused / updated in one place.
class PHOTONCORE_EXPORT FixtureGroup
{
public:
    QString      name;
    FixtureQuery query;

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;
};

class PHOTONCORE_EXPORT FixtureGroupCollection : public QObject
{
    Q_OBJECT
public:
    explicit FixtureGroupCollection(QObject *parent = nullptr);
    ~FixtureGroupCollection();

    int groupCount() const;
    FixtureGroup *groupAtIndex(int) const;
    FixtureGroup *findGroup(const QString &name) const;
    QStringList groupNames() const;

    void clear();

signals:
    void groupWillBeAdded(int);
    void groupWasAdded(int);
    void groupWillBeRemoved(int);
    void groupWasRemoved(int);
    void groupChanged(int);

public slots:
    FixtureGroup *addGroup(const QString &name);
    void removeGroup(const QString &name);
    // Notify listeners (e.g. the panel model) that a group's contents changed.
    void notifyChanged(FixtureGroup *);

public:
    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREGROUP_H
