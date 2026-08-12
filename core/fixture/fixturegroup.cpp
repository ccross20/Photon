#include "fixturegroup.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QVector>
#include "fixturegroupeditor.h"

namespace photon {

FixtureGroup::FixtureGroup(const QString &t_name, QObject *parent)
    : QObject{parent}, m_name(t_name)
{
    m_uniqueId = QUuid::createUuid().toByteArray();
}

QString FixtureGroup::name() const
{
    return m_name;
}

void FixtureGroup::setName(const QString &t_name)
{
    if(m_name == t_name)
        return;
    m_name = t_name;
    notifyResourceChanged();
    emit groupModified(this);
}

QByteArray FixtureGroup::uniqueId() const
{
    return m_uniqueId;
}

const FixtureQuery &FixtureGroup::query() const
{
    return m_query;
}

void FixtureGroup::setQuery(const FixtureQuery &t_query)
{
    m_query = t_query;
    notifyResourceChanged();
    emit groupModified(this);
}

QWidget *FixtureGroup::createResourceEditor()
{
    return new FixtureGroupEditor(this);
}

void FixtureGroup::readFromJson(const QJsonObject &json)
{
    m_name = json.value("name").toString();
    if(json.contains("uniqueId"))
        m_uniqueId = json.value("uniqueId").toString().toLatin1();
    m_query.readFromJson(json.value("query").toObject());
    readResourceJson(json);
}

void FixtureGroup::writeToJson(QJsonObject &json) const
{
    json.insert("name", m_name);
    json.insert("uniqueId", QString(m_uniqueId));
    QJsonObject queryObj;
    m_query.writeToJson(queryObj);
    json.insert("query", queryObj);
    writeResourceJson(json);
}

class FixtureGroupCollection::Impl
{
public:
    QVector<FixtureGroup *> groups;

    int indexOf(const QString &name) const
    {
        for(int i = 0; i < groups.size(); ++i)
            if(groups[i]->name().compare(name, Qt::CaseInsensitive) == 0)
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

FixtureGroup *FixtureGroupCollection::findGroupWithId(const QByteArray &t_id) const
{
    for(FixtureGroup *g : m_impl->groups)
        if(g->uniqueId() == t_id)
            return g;
    return nullptr;
}

const QVector<FixtureGroup*> &FixtureGroupCollection::groups() const
{
    return m_impl->groups;
}

QStringList FixtureGroupCollection::groupNames() const
{
    QStringList names;
    for(FixtureGroup *g : m_impl->groups)
        names << g->name();
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
    auto *group = new FixtureGroup(name);
    emit groupWillBeAdded(index);
    emit groupWillBeAddedAt(group, index);
    m_impl->groups.append(group);
    connect(group, &FixtureGroup::groupModified, this, &FixtureGroupCollection::notifyChanged);
    emit groupWasAdded(index);
    emit groupWasAddedAt(group, index);
    return group;
}

void FixtureGroupCollection::removeGroup(const QString &name)
{
    removeGroup(findGroup(name));
}

void FixtureGroupCollection::removeGroup(photon::FixtureGroup *t_group)
{
    if(!t_group)
        return;

    const int index = m_impl->groups.indexOf(t_group);
    if(index < 0)
        return;

    emit groupWillBeRemoved(index);
    emit groupWillBeRemovedAt(t_group, index);
    m_impl->groups.removeAt(index);
    emit groupWasRemoved(index);
    emit groupWasRemovedAt(t_group, index);

    t_group->deleteLater();
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
        connect(group, &FixtureGroup::groupModified, this, &FixtureGroupCollection::notifyChanged);
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
