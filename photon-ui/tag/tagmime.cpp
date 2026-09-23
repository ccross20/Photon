#include <QMimeData>
#include <QCoreApplication>
#include <QDataStream>
#include <QIODevice>
#include "tagmime.h"

namespace photon {

const QByteArray TagMimeType = "photon.core.tag";

QMimeData *encodeTagMime(const QStringList &t_tags)
{
    QMimeData *mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << QCoreApplication::applicationPid();
    stream << t_tags;

    mimeData->setData(TagMimeType, data);
    return mimeData;
}

QStringList decodeTagMime(const QMimeData *t_mimeData)
{
    if(!t_mimeData || !t_mimeData->hasFormat(TagMimeType))
        return {};

    QByteArray data = t_mimeData->data(TagMimeType);
    QDataStream stream(&data, QIODevice::ReadOnly);

    qint64 senderPid = 0;
    stream >> senderPid;
    if(senderPid != QCoreApplication::applicationPid())
        return {};

    QStringList tags;
    stream >> tags;
    return tags;
}

} // namespace photon
