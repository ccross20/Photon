#ifndef PHOTON_FIXTUREGROUP_H
#define PHOTON_FIXTUREGROUP_H

#include <QObject>
#include <QStringList>
#include "photon-global.h"
#include "fixture/fixturequery.h"
#include "project/projectresource.h"

class QJsonObject;

namespace photon {

// A named, saved fixture selection (a FixtureQuery with a name). Referenced by name
// from shows so a selection can be defined once and reused / updated in one place.
class PHOTONCORE_EXPORT FixtureGroup : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    explicit FixtureGroup(const QString &name = QString{}, QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &);
    QByteArray uniqueId() const;

    const FixtureQuery &query() const;
    void setQuery(const FixtureQuery &);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "group";}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override;

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

signals:
    // The group's name or query changed. FixtureGroupCollection relays this as
    // its index-based groupChanged().
    void groupModified(photon::FixtureGroup *);

private:
    QString m_name;
    QByteArray m_uniqueId;
    FixtureQuery m_query;
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
    FixtureGroup *findGroupWithId(const QByteArray &) const;
    const QVector<FixtureGroup*> &groups() const;
    QStringList groupNames() const;

    void clear();

signals:
    void groupWillBeAdded(int);
    void groupWasAdded(int);
    void groupWillBeRemoved(int);
    void groupWasRemoved(int);
    void groupChanged(int);

    // Pointer-based equivalents of the four above, matching the shape every
    // other collection uses so one model can consume them uniformly.
    void groupWillBeAddedAt(photon::FixtureGroup *, int);
    void groupWasAddedAt(photon::FixtureGroup *, int);
    void groupWillBeRemovedAt(photon::FixtureGroup *, int);
    void groupWasRemovedAt(photon::FixtureGroup *, int);

public slots:
    FixtureGroup *addGroup(const QString &name);
    void removeGroup(const QString &name);
    void removeGroup(photon::FixtureGroup *);
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
