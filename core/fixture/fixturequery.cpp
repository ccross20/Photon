#include "fixturequery.h"
#include <QJsonObject>
#include <QJsonArray>
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "scene/scenezone.h"
#include "project/project.h"

namespace photon {

bool FixtureQuery::matches(Fixture *fixture) const
{
    if(!fixture)
        return false;

    // Tags: every requested tag must be present (AND), case-insensitive.
    const QStringList have = fixture->tags();
    for(const QString &tag : tags)
    {
        bool found = false;
        for(const QString &h : have)
        {
            if(h.compare(tag, Qt::CaseInsensitive) == 0) { found = true; break; }
        }
        if(!found)
            return false;
    }

    // Type: case-insensitive substring of the model type or any OpenFixture category.
    const QString t = type.trimmed();
    if(!t.isEmpty())
    {
        bool typeOk = fixture->modelType().contains(t, Qt::CaseInsensitive);
        if(!typeOk)
        {
            for(const QString &cat : fixture->categories())
            {
                if(cat.contains(t, Qt::CaseInsensitive)) { typeOk = true; break; }
            }
        }
        if(!typeOk)
            return false;
    }

    return true;
}

QVector<Fixture *> FixtureQuery::resolve(Project *project) const
{
    QVector<Fixture *> result;
    if(!project || isEmpty())
        return result;   // no predicates = no selection (not "everything")

    // Spatial predicate: resolve the named zone once, then test fixture positions.
    SceneZone *zoneObj = nullptr;
    const QString zoneName = zone.trimmed();
    if(!zoneName.isEmpty())
    {
        zoneObj = SceneZone::findByName(project, zoneName);
        if(!zoneObj)
            return result;   // a named-but-missing zone matches nothing
    }

    for(Fixture *fixture : project->fixtures()->fixtures())
    {
        if(!matches(fixture))
            continue;
        if(zoneObj && !zoneObj->containsPoint(fixture->globalPosition()))
            continue;
        result.append(fixture);
    }
    return result;
}

void FixtureQuery::readFromJson(const QJsonObject &json)
{
    tags.clear();
    const QJsonArray tagArray = json.value("tags").toArray();
    for(const auto &t : tagArray)
        tags << t.toString();
    type = json.value("type").toString();
    zone = json.value("zone").toString();
}

void FixtureQuery::writeToJson(QJsonObject &json) const
{
    QJsonArray tagArray;
    for(const QString &t : tags)
        tagArray.append(t);
    json.insert("tags", tagArray);
    json.insert("type", type);
    json.insert("zone", zone);
}

} // namespace photon
