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
class QJsonObject;

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

    // Whether the plugin is currently connected - outbound commands are silently
    // dropped when false.
    bool isConnected() const { return socket != nullptr; }

signals:
    // Emitted once per parsed telemetry line, after the public fields below have
    // been updated - lets a consumer (e.g. a live capture) react per-tick instead of
    // polling on its own timer.
    void dataUpdated();

public slots:
    // Outbound transport control, sent to the plugin over the same TCP socket it
    // streams telemetry on. Translated into VDJScript verbs on the plugin side
    // (SendCommand). No-ops (silently) when not connected.
    void sendPlay();
    void sendPause();
    void sendRestart();          // jump to the start of the track
    void sendSeek(double seconds);
    void sendCaptureStart();     // ask the plugin to tick faster while we record beats
    void sendCaptureStop();

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
    // Level/stem telemetry, from VDJ's own engine (get_level, overall and per-stem)
    // - nominally 0..1, unverified against a live session. Stem names match this
    // install's Stems 2.0 UI labels.
    double level = 0.0;
    double stemVocal = 0.0;
    double stemInstru = 0.0;
    double stemBass = 0.0;
    double stemKick = 0.0;
    double stemHiHat = 0.0;
    QString path;
    QString title;
    QString artist;
    QString sample1;
    QString activeDeck;
    QVector<QueueEntry> queue;

private:
    void initServer();
    void processLine(const QByteArray &line);
    void sendCommand(const QJsonObject &command);

    QTcpServer *tcpServer = nullptr;
    QTcpSocket *socket = nullptr;
    QUdpSocket *discoverySocket = nullptr;
    QTimer *discoveryTimer = nullptr;
    QByteArray m_buffer;   // accumulates partial lines between reads
};

} // namespace photon

#endif // VIRTUALDJCONNECTOR_H
