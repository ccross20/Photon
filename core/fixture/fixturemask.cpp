#include "fixturemask.h"

namespace photon {

class FixtureMask::Impl
{
public:
    QSet<Fixture*> fixtures;
};

FixtureMask::FixtureMask(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

FixtureMask::~FixtureMask()
{
    delete m_impl;
}

void FixtureMask::addFixture(Fixture *t_fixture)
{
    if(m_impl->fixtures.contains(t_fixture))
        return;
    m_impl->fixtures.insert(t_fixture);
    emit FixtureAdded(t_fixture);
}

void FixtureMask::removeFixture(Fixture *t_fixture)
{
    if(m_impl->fixtures.remove(t_fixture))
        emit FixtureRemoved(t_fixture);
}

const QSet<Fixture*> &FixtureMask::fixtures() const
{
    return m_impl->fixtures;
}

} // namespace photon
