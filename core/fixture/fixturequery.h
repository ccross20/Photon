#ifndef PHOTON_FIXTUREQUERY_H
#define PHOTON_FIXTUREQUERY_H

#include <QStringList>
#include <QVector>
#include "photon-global.h"

class QJsonObject;

namespace photon {

// A dynamic fixture selection: predicates over tags (AND) and type. Resolved live
// against the project, so the matching set tracks rig changes (added / retagged
// fixtures) without editing whatever referenced the query.
class PHOTONCORE_EXPORT FixtureQuery
{
public:
    QStringList tags;   // a fixture must carry ALL of these (case-insensitive)
    QString     type;   // case-insensitive substring of modelType or any category
    QString     zone;   // name of a SceneZone the fixture must sit inside

    bool matches(Fixture *fixture) const;   // tag + type only (zone needs the project)
    QVector<Fixture *> resolve(Project *project) const;

    bool isEmpty() const
    {
        return tags.isEmpty() && type.trimmed().isEmpty() && zone.trimmed().isEmpty();
    }

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;
};

} // namespace photon

#endif // PHOTON_FIXTUREQUERY_H
