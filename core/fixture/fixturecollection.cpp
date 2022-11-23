#include "fixturecollection.h"
#include "fixture.h"

namespace photon {

class FixtureCollection::Impl
{
public:
    QVector<Fixture*> fixtures;
};

FixtureCollection::FixtureCollection(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

FixtureCollection::~FixtureCollection(){
    for(auto fixture : m_impl->fixtures)
        delete fixture;
    delete m_impl;
}

void FixtureCollection::addFixture(Fixture *t_fixture)
{
    if(m_impl->fixtures.contains(t_fixture))
        return;
    emit fixtureWillBeAdded(t_fixture, m_impl->fixtures.length());
    m_impl->fixtures.append(t_fixture);
    emit fixtureWasAdded(t_fixture);

}

Fixture *FixtureCollection::fixtureAtIndex(uint t_index) const
{
    return m_impl->fixtures.at(t_index);
}

int FixtureCollection::fixtureCount() const
{
    return m_impl->fixtures.length();
}

QVector<Fixture*> FixtureCollection::fixturesWithName(const QString &t_text)
{
    QVector<Fixture*> results;

    for(auto it = m_impl->fixtures.crbegin(); it != m_impl->fixtures.crend(); ++it)
    {
        auto fix = *it;
        if(fix->name() == t_text)
            results.append(fix);
    }

    return results;

}

void FixtureCollection::removeAllFixtures()
{
    int counter = fixtureCount()-1;
    for(auto it = m_impl->fixtures.crbegin(); it != m_impl->fixtures.crend(); ++it)
    {
        auto fix = *it;
        emit fixtureWillBeRemoved(fix,counter--);
        m_impl->fixtures.removeOne(fix);
        emit fixtureWasRemoved(fix);
    }
}

void FixtureCollection::removeFixture(Fixture *t_fixture)
{
    if(!m_impl->fixtures.contains(t_fixture))
        return;

    int index = 0;
    for(auto fix : m_impl->fixtures)
    {
        if(fix == t_fixture)
            break;
        ++index;
    }

    emit fixtureWillBeRemoved(t_fixture, index);
    m_impl->fixtures.removeOne(t_fixture);
    emit fixtureWasRemoved(t_fixture);
}

const QVector<Fixture*> &FixtureCollection::fixtures() const
{
    return m_impl->fixtures;
}

} // namespace photon
