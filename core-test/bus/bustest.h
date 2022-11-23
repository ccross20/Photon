#ifndef PHOTON_BUSTEST_H
#define PHOTON_BUSTEST_H

#include <QObject>

namespace photon {

class BusTest : public QObject
{
    Q_OBJECT
public:
    explicit BusTest(QObject *parent = nullptr);

private slots:
    void readDMX();

};

} // namespace photon

#endif // PHOTON_BUSTEST_H
