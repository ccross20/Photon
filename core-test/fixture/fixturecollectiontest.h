#ifndef PHOTON_FIXTURECOLLECTIONTEST_H
#define PHOTON_FIXTURECOLLECTIONTEST_H

#include <QObject>

namespace photon {

class FixtureCollectionTest : public QObject
{
    Q_OBJECT
public:
    explicit FixtureCollectionTest(QObject *parent = nullptr);

signals:

private slots:
    void addFixture();
    void removeFixture();
    void findFixture();

};

} // namespace photon

#endif // PHOTON_FIXTURECOLLECTIONTEST_H
