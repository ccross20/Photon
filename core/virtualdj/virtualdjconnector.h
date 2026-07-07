#ifndef VIRTUALDJCONNECTOR_H
#define VIRTUALDJCONNECTOR_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QByteArray>
#include "photon-global.h"
class QTcpServer;
class QTcpSocket;
class QUdpSocket;
class QTimer;

namespace photon {

class PHOTONCORE_EXPORT VirtualDJConnector : public QObject
{
    Q_OBJECT
public:
    VirtualDJConnector();

    // One upcoming song in VirtualDJ's automix queue.
    struct QueueEntry {
        QString title;
        QString artist;
        double  bpm = 0.0;
    };

private slots:
    void newConnection();
    void readReady();
    void broadcastDiscovery();

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
    double firstBeat = 0.0;
    double songLength = 0.0;
    double sample1Pos = 0.0;
    QString path;
    QString title;
    QString artist;
    QString sample1;
    QString activeDeck;
    QVector<QueueEntry> queue;

private:
    void initServer();
    void processLine(const QByteArray &line);

    QTcpServer *tcpServer = nullptr;
    QTcpSocket *socket = nullptr;
    QUdpSocket *discoverySocket = nullptr;
    QTimer *discoveryTimer = nullptr;
    QByteArray m_buffer;   // accumulates partial lines between reads
};

} // namespace photon

#endif // VIRTUALDJCONNECTOR_H
