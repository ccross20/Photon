#ifndef PHOTON_FIXTURECAPABILITYTEST_H
#define PHOTON_FIXTURECAPABILITYTEST_H

#include <QObject>


namespace photon {

class FixtureCapabilityTest : public QObject
{
    Q_OBJECT

public:
    FixtureCapabilityTest(QObject *parent = nullptr);

private slots:
    void unitTest();
};

} // namespace photon

#endif // PHOTON_FIXTURECAPABILITYTEST_H
