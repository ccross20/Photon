#include <QtNetwork>
#include <QJsonDocument>
#include "virtualdjconnector.h"

namespace photon {

VirtualDJConnector::VirtualDJConnector() : QObject() {
    initServer();

    connect(tcpServer, &QTcpServer::newConnection, this, &VirtualDJConnector::newConnection);
}


void VirtualDJConnector::initServer()
{
    //! [0] //! [1]
    tcpServer = new QTcpServer(this);

    if (!tcpServer->listen(QHostAddress::Any, 6789)) {
        qDebug() << "Unable to start the server: " << tcpServer->errorString();


        return;
    }
    //! [0]
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    qDebug() << QString("The server is running on\n\nIP: %1\nport: %2\n\n").arg(ipAddress).arg(tcpServer->serverPort());

    //! [1]
}

void VirtualDJConnector::newConnection()
{
    socket = tcpServer->nextPendingConnection();

    connect(socket, &QTcpSocket::readyRead, this, &VirtualDJConnector::readReady);
}

void VirtualDJConnector::readReady()
{
    int toRead = socket->bytesAvailable();


    QByteArray data = socket->read(toRead).trimmed();

    //qDebug() << data;


    if(!data.isEmpty())
    {
        QJsonDocument doc = QJsonDocument::fromJson(data);

        auto obj = doc.object();


        //qDebug() << data;


        if(obj.contains("time"))
            time = obj.value("time").toDouble();
        if(obj.contains("progress"))
            progress =  obj.value("progress").toDouble();
        if(obj.contains("path"))
        {
            QString p = obj.value("path").toString();
            if(p != path)
            {
                path = p;
                qDebug() << "Path updated: " << p;
            }
        }
        if(obj.contains("beatPos"))
        {
            double beatProgressOrig = obj.value("beatPos").toDouble();
            beatProgress = beatProgressOrig;
            beatNumber = floor(beatProgress);
            beatProgress -= beatNumber;

            if(beatProgressOrig > 0)
            {
                beatProgress4 = ((beatNumber % 4) + beatProgress)/4.0;
                beatProgress2 = ((beatNumber % 2) + beatProgress)/2.0;

                beatProgress4 = abs((beatProgress4 - .5)*2.0);
                beatProgress2 = abs((beatProgress2 - .5)*2.0);
            }
            else
            {
                beatProgress4 = 0.0;
                beatProgress2 = 0.0;

            }
            beatAmount = abs((beatProgress - .5)*2.0);

        }
        if(obj.contains("bpm"))
        {
            bpm =obj.value("bpm").toDouble();\
        }
        if(obj.contains("beatIntensity"))
            beatIntensity =obj.value("beatIntensity").toDouble();

        if(obj.contains("nextbeat"))
            timeOfNextBeat = obj.value("nextbeat").toDouble();

    }

}

} // namespace photon
