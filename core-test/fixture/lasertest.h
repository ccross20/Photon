#ifndef PHOTON_LASERTEST_H
#define PHOTON_LASERTEST_H

#include <QObject>

namespace photon {

class LaserTest : public QObject
{
    Q_OBJECT

public:
    LaserTest(QObject *parent = nullptr);

private slots:
    void definitionLoads();
    void neutralValues();
    void disarmedIsSafe();
    void armedAndSetupModes();
    void rotationWords();
    void contentAndStrobe();
    void centeredExtremes();
    void positionPointSaves();
};

} // namespace photon

#endif // PHOTON_LASERTEST_H
