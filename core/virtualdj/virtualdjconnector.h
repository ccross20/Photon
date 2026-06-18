#ifndef VIRTUALDJCONNECTOR_H
#define VIRTUALDJCONNECTOR_H

#include <QObject>
#include "photon-global.h"
class QTcpServer;
class QTcpSocket;

namespace photon {

class PHOTONCORE_EXPORT VirtualDJConnector : public QObject
{
    Q_OBJECT
public:
    VirtualDJConnector();

private slots:
    void newConnection();
    void readReady();

public:
    double time = 0.0;
    double progress = 0.0;
    double timeOfNextBeat = 0.0;
    double beatProgress = 0.0;
    double beatAmount = 0.0;
    double beatIntensity = 0.0;
    double beatProgress2 = 0.0;
    double beatProgress4 = 0.0;
    int beatNumber = 0;
    double bpm = 0.0;
    QString path;

private:
    void initServer();

    QTcpServer *tcpServer = nullptr;
    QTcpSocket *socket;


};

} // namespace photon

#endif // VIRTUALDJCONNECTOR_H
