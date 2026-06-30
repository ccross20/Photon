#include "fixturegroup.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

namespace photon {

void FixtureGroup::readFromJson(const QJsonObject &json)
{
    name = json.value("name").toString();
    query.readFromJson(json.value("query").toObject());
}

void FixtureGroup::writeToJson(QJsonObject &json) const
{
    json.insert("name", name);
    QJsonObject queryObj;
    query.writeToJson(queryObj);
    json.insert("query", queryObj);
}

class FixtureGroupCollection::Impl
{
public:
    QVector<FixtureGroup *> groups;

    int indexOf(const QString &name) const
    {
        for(int i = 0; i < groups.size(); ++i)
            if(groups[i]->name.compare(name, Qt::CaseInsensitive) == 0)
                return i;
        return -1;
    }
};

FixtureGroupCollection::FixtureGroupCollection(QObject *parent) : QObject(parent), m_impl(new Impl)
{
}

FixtureGroupCollection::~FixtureGroupCollection()
{
    clear();
    delete m_impl;
}

int FixtureGroupCollection::groupCount() const
{
    return m_impl->groups.size();
}

FixtureGroup *FixtureGroupCollection::groupAtIndex(int t_index) const
{
    if(t_index < 0 || t_index >= m_impl->groups.size())
        return nullptr;
    return m_impl->groups[t_index];
}

FixtureGroup *FixtureGroupCollection::findGroup(const QString &name) const
{
    const int idx = m_impl->indexOf(name);
    return idx >= 0 ? m_impl->groups[idx] : nullptr;
}

QStringList FixtureGroupCollection::groupNames() const
{
    QStringList names;
    for(FixtureGroup *g : m_impl->groups)
        names << g->name;
    return names;
}

void FixtureGroupCollection::clear()
{
    qDeleteAll(m_impl->groups);
    m_impl->groups.clear();
}

FixtureGroup *FixtureGroupCollection::addGroup(const QString &name)
{
    if(FixtureGroup *existing = findGroup(name))
        return existing;

    const int index = m_impl->groups.size();
    emit groupWillBeAdded(index);
    auto *group = new FixtureGroup;
    group->name = name;
    m_impl->groups.append(group);
    emit groupWasAdded(index);
    return group;
}

void FixtureGroupCollection::removeGroup(const QString &name)
{
    const int index = m_impl->indexOf(name);
    if(index < 0)
        return;
    emit groupWillBeRemoved(index);
    delete m_impl->groups.takeAt(index);
    emit groupWasRemoved(index);
}

void FixtureGroupCollection::notifyChanged(FixtureGroup *group)
{
    const int index = m_impl->groups.indexOf(group);
    if(index >= 0)
        emit groupChanged(index);
}

void FixtureGroupCollection::readFromJson(const QJsonObject &json)
{
    clear();
    const QJsonArray groupArray = json.value("groups").toArray();
    for(const auto &g : groupArray)
    {
        auto *group = new FixtureGroup;
        group->readFromJson(g.toObject());
        m_impl->groups.append(group);
    }
}

void FixtureGroupCollection::writeToJson(QJsonObject &json) const
{
    QJsonArray groupArray;
    for(FixtureGroup *g : m_impl->groups)
    {
        QJsonObject groupObj;
        g->writeToJson(groupObj);
        groupArray.append(groupObj);
    }
    json.insert("groups", groupArray);
}

} // namespace photon
