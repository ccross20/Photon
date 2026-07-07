#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <cmath>
#include "virtualdjconnector.h"

namespace photon {

namespace {
    // Must match the ConnectionPlugin (VirtualDJ side).
    constexpr quint16 kServerPort    = 6789;                     // TCP: plugin -> Photon data
    constexpr quint16 kDiscoveryPort = 6790;                     // UDP: discovery beacon
    const QByteArray  kDiscoveryPrefix = "VDJCONNECTOR_DISCOVER:";
    constexpr int     kDiscoveryIntervalMs = 2000;
}

VirtualDJConnector::VirtualDJConnector() : QObject() {
    initServer();

    connect(tcpServer, &QTcpServer::newConnection, this, &VirtualDJConnector::newConnection);

    // The plugin defaults to "auto": it listens on UDP kDiscoveryPort for our beacon
    // and connects back to us. Broadcast it periodically so the plugin (possibly on
    // another machine) can find this server without hard-coding an IP.
    discoverySocket = new QUdpSocket(this);
    discoveryTimer = new QTimer(this);
    connect(discoveryTimer, &QTimer::timeout, this, &VirtualDJConnector::broadcastDiscovery);
    discoveryTimer->start(kDiscoveryIntervalMs);
    broadcastDiscovery();   // send one immediately
}


void VirtualDJConnector::initServer()
{
    tcpServer = new QTcpServer(this);

    if (!tcpServer->listen(QHostAddress::Any, kServerPort)) {
        qWarning() << "VirtualDJConnector: unable to start server:" << tcpServer->errorString();
        return;
    }

    QString ipAddress;
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : ipAddressesList) {
        if (addr != QHostAddress::LocalHost && addr.toIPv4Address()) {
            ipAddress = addr.toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    qWarning().noquote() << QString("VirtualDJConnector: listening on %1:%2")
                                .arg(ipAddress).arg(tcpServer->serverPort());
}

void VirtualDJConnector::broadcastDiscovery()
{
    if (!discoverySocket || !tcpServer || !tcpServer->isListening())
        return;

    const QByteArray msg = kDiscoveryPrefix + QByteArray::number(tcpServer->serverPort());

    // Global broadcast plus each interface's subnet broadcast. The per-interface sends
    // make the plugin see the correct source IP for the network it's reachable on.
    discoverySocket->writeDatagram(msg, QHostAddress::Broadcast, kDiscoveryPort);
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!iface.flags().testFlag(QNetworkInterface::IsUp)) continue;
        if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
            const QHostAddress bc = entry.broadcast();
            if (!bc.isNull())
                discoverySocket->writeDatagram(msg, bc, kDiscoveryPort);
        }
    }
}

void VirtualDJConnector::newConnection()
{
    QTcpSocket *incoming = tcpServer->nextPendingConnection();
    if (!incoming)
        return;

    // The plugin reconnects on error, so drop any previous connection cleanly.
    if (socket) {
        socket->disconnect(this);
        socket->deleteLater();
    }
    socket = incoming;
    m_buffer.clear();

    connect(socket, &QTcpSocket::readyRead, this, &VirtualDJConnector::readReady);
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QTcpSocket::disconnected, this, [this]() { socket = nullptr; });
}

void VirtualDJConnector::readReady()
{
    if (!socket)
        return;

    // The plugin streams one JSON object per line at ~60ms; a single read can hold
    // zero, one, or several lines (and a trailing partial). Frame on newlines.
    m_buffer.append(socket->readAll());

    int nl;
    while ((nl = m_buffer.indexOf('\n')) >= 0) {
        const QByteArray line = m_buffer.left(nl).trimmed();
        m_buffer.remove(0, nl + 1);
        if (!line.isEmpty())
            processLine(line);
    }
}

void VirtualDJConnector::processLine(const QByteArray &line)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    const QJsonObject obj = doc.object();

    if (obj.contains("time"))
        time = obj.value("time").toDouble();
    if (obj.contains("progress"))
        progress = obj.value("progress").toDouble();
    if (obj.contains("bpm"))
        bpm = obj.value("bpm").toDouble();
    if (obj.contains("beatIntensity"))
        beatIntensity = obj.value("beatIntensity").toDouble();
    if (obj.contains("nextbeat"))
        timeOfNextBeat = obj.value("nextbeat").toDouble();
    if (obj.contains("firstbeat"))
        firstBeat = obj.value("firstbeat").toDouble();
    if (obj.contains("songlength"))
        songLength = obj.value("songlength").toDouble();
    if (obj.contains("sample1Pos"))
        sample1Pos = obj.value("sample1Pos").toDouble();

    if (obj.contains("path")) {
        const QString p = obj.value("path").toString();
        if (p != path) {
            path = p;
            qWarning() << "VirtualDJConnector: track changed:" << p;
        }
    }
    if (obj.contains("title"))
        title = obj.value("title").toString();
    if (obj.contains("artist"))
        artist = obj.value("artist").toString();
    if (obj.contains("sample1"))
        sample1 = obj.value("sample1").toString();
    if (obj.contains("activedeck"))
        activeDeck = obj.value("activedeck").toString();

    if (obj.contains("beatPos")) {
        const double beatProgressOrig = obj.value("beatPos").toDouble();
        beatProgress = beatProgressOrig;
        beatNumber = int(std::floor(beatProgress));
        beatProgress -= beatNumber;

        if (beatProgressOrig > 0) {
            beatProgress4 = ((beatNumber % 4) + beatProgress) / 4.0;
            beatProgress2 = ((beatNumber % 2) + beatProgress) / 2.0;
            beatProgress4 = std::abs((beatProgress4 - 0.5) * 2.0);
            beatProgress2 = std::abs((beatProgress2 - 0.5) * 2.0);
        } else {
            beatProgress4 = 0.0;
            beatProgress2 = 0.0;
        }
        beatAmount = std::abs((beatProgress - 0.5) * 2.0);
    }

    if (obj.contains("queue")) {
        queue.clear();
        const QJsonArray arr = obj.value("queue").toArray();
        for (const QJsonValue &v : arr) {
            const QJsonObject e = v.toObject();
            QueueEntry entry;
            entry.title = e.value("title").toString();
            entry.artist = e.value("artist").toString();
            entry.bpm = e.value("bpm").toDouble();
            queue.append(entry);
        }
    }
}

} // namespace photon
