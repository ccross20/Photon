#ifndef PHOTON_FALLOFFTEST_H
#define PHOTON_FALLOFFTEST_H

#include <QObject>

namespace photon {

class FalloffTest : public QObject
{
    Q_OBJECT

private slots:
    void linear();
    void wrapModes();
    void mirroring();
    void radialIgnoresHeight();
    void conicalSweep();
    void worldTransform();
    void loadsOldLinearFalloff();
};

} // namespace photon

#endif // PHOTON_FALLOFFTEST_H
