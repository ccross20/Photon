#include <QTransform>
#include "falloffmap2d.h"

namespace photon {

class FalloffMap2D::Impl
{
public:
    void rebuildTransform();
    QVector<Falloff2DData> positions;
    QTransform transform;
    QPointF startPt;
    QPointF endPt;
};

void FalloffMap2D::Impl::rebuildTransform()
{
    transform.reset();

    QPointF delta = endPt - startPt;
    double length = QLineF(QPointF(), delta).length();
    transform.translate(startPt.x(), startPt.y());
    transform.scale(length, length);
    transform.rotate(qRadiansToDegrees(atan2(delta.y(), delta.x())));

    transform = transform.inverted();

}

FalloffMap2D::FalloffMap2D() : FixtureFalloffMap(),m_impl(new Impl)
{
    m_impl->startPt = QPointF(0,.5);
    m_impl->endPt = QPointF(1,.5);
    m_impl->rebuildTransform();
}

FalloffMap2D::FalloffMap2D(const FalloffMap2D &other)
{
    setFixtureData(other.fixtureData());
}

FalloffMap2D::~FalloffMap2D()
{
    delete m_impl;
}

FalloffMap2D &FalloffMap2D::operator=(const FalloffMap2D &other)
{
    setFixtureData(other.fixtureData());
    return *this;
}

void FalloffMap2D::addFixture(Fixture *t_fixture, const QPointF &t_position)
{
    Falloff2DData d;
    d.fixture = t_fixture;
    d.position = t_position;
    m_impl->positions.append(d);
}

void FalloffMap2D::removeFixture(Fixture *t_fixture)
{
    for(auto it = m_impl->positions.cbegin(); it != m_impl->positions.cend(); ++it)
    {
        if((*it).fixture == t_fixture)
        {
            m_impl->positions.erase(it);
            return;
        }
    }
}

void FalloffMap2D::setFixturePosition(Fixture *t_fixture, const QPointF &t_position)
{
    for(auto &data : m_impl->positions)
    {
        if(data.fixture == t_fixture)
        {
            data.position = t_position;
        }
    }
}

QPointF FalloffMap2D::fixturePosition(Fixture *t_fixture) const
{
    for(const auto &data : m_impl->positions)
    {
        if(data.fixture == t_fixture)
        {
            return data.position;
        }
    }

    return QPointF{};
}

double FalloffMap2D::falloff(Fixture *t_fixture)
{
    for(const auto &data : m_impl->positions)
    {
        if(data.fixture == t_fixture)
        {
            return std::max(std::min(m_impl->transform.map(data.position).x(), 1.0), 0.0);
        }
    }

    return 0;
}

QVector<FalloffMap2D::Falloff2DData> FalloffMap2D::fixtureData() const
{
    return m_impl->positions;
}

void FalloffMap2D::setFixtureData(const QVector<Falloff2DData> &t_positions)
{
    m_impl->positions = t_positions;
}

QPointF FalloffMap2D::startPosition() const
{
    return m_impl->startPt;
}

QPointF FalloffMap2D::endPosition() const
{
    return m_impl->endPt;
}

void FalloffMap2D::setStartPosition(const QPointF &t_pt)
{
    m_impl->startPt = t_pt;
    m_impl->rebuildTransform();
}

void FalloffMap2D::setEndPosition(const QPointF &t_pt)
{
    m_impl->endPt = t_pt;
    m_impl->rebuildTransform();
}



} // namespace photon
