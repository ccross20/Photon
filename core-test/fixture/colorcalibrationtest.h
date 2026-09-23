#ifndef PHOTON_COLORCALIBRATIONTEST_H
#define PHOTON_COLORCALIBRATIONTEST_H

#include <QObject>

namespace photon {

class ColorCalibrationTest : public QObject
{
    Q_OBJECT

public:
    ColorCalibrationTest(QObject *parent = nullptr);

private slots:
    void unitTest();
};

} // namespace photon

#endif // PHOTON_COLORCALIBRATIONTEST_H
