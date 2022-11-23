#ifndef PHOTON_FALLOFFMAP2D_H
#define PHOTON_FALLOFFMAP2D_H

#include "fixturefalloffmap.h"

namespace photon {

class FalloffMap2D : public FixtureFalloffMap
{
public:

    struct Falloff2DData{
        Fixture *fixture;
        QPointF position;
    };

    FalloffMap2D();
    FalloffMap2D(const FalloffMap2D &other);
    ~FalloffMap2D();

    FalloffMap2D &operator=(const FalloffMap2D &other);

    void addFixture(Fixture *fixture, const QPointF &);
    void removeFixture(Fixture *);
    void setFixturePosition(Fixture *fixture, const QPointF &);
    QPointF fixturePosition(Fixture *) const;
    double falloff(Fixture *) override;

    QVector<Falloff2DData> fixtureData() const;
    void setFixtureData(const QVector<Falloff2DData> &);

    QPointF startPosition() const;
    QPointF endPosition() const;
    void setStartPosition(const QPointF &);
    void setEndPosition(const QPointF &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FALLOFFMAP2D_H
