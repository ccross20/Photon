#include "fixturefalloffmap.h"

namespace photon {

class FixtureFalloffMap::Impl
{
public:
    QHash<Fixture*,double> hash;
};

FixtureFalloffMap::FixtureFalloffMap(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

FixtureFalloffMap::~FixtureFalloffMap()
{
    delete m_impl;
}

void FixtureFalloffMap::addFixture(Fixture *fixture, double delay)
{
    m_impl->hash.insert(fixture, delay);

    emit Updated();
}

double FixtureFalloffMap::falloff(Fixture *t_fixture)
{
    return m_impl->hash.value(t_fixture, 0.0);
}

} // namespace photon
