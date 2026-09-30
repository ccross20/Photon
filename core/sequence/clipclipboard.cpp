#include <limits>
#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include "clipclipboard.h"
#include "clip.h"
#include "cliplayer.h"
#include "sequence.h"

namespace photon {

namespace ClipClipboard {

namespace {
const QString kClipMime = QStringLiteral("application/x-photon-clips");
}

void copy(const QVector<Clip *> &t_clips)
{
    if(t_clips.isEmpty())
        return;

    // Layer offsets are relative to the topmost copied layer.
    int topLayerIndex = std::numeric_limits<int>::max();
    for(Clip *clip : t_clips)
        if(clip->sequence() && clip->layer())
            topLayerIndex = std::min(topLayerIndex, int(clip->sequence()->layers().indexOf(clip->layer())));

    QJsonArray entries;
    for(Clip *clip : t_clips)
    {
        QJsonObject clipJson;
        clip->writeToJson(clipJson);

        int layerIndex = topLayerIndex;
        if(clip->sequence() && clip->layer())
            layerIndex = clip->sequence()->layers().indexOf(clip->layer());

        QJsonObject entry;
        entry.insert("layerOffset", std::max(0, layerIndex - topLayerIndex));
        entry.insert("clip", clipJson);
        entries.append(entry);
    }

    auto *mime = new QMimeData;
    mime->setData(kClipMime, QJsonDocument(QJsonObject{{"clips", entries}}).toJson(QJsonDocument::Compact));
    QGuiApplication::clipboard()->setMimeData(mime);
}

bool hasClips()
{
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    return mime && mime->hasFormat(kClipMime);
}

QVector<Clip *> paste(Sequence *t_sequence, ClipLayer *t_targetLayer, double t_time)
{
    QVector<Clip *> pasted;
    if(!t_sequence || !t_targetLayer || !hasClips())
        return pasted;

    const QJsonArray entries = QJsonDocument::fromJson(
        QGuiApplication::clipboard()->mimeData()->data(kClipMime)).object().value("clips").toArray();
    if(entries.isEmpty())
        return pasted;

    double earliest = std::numeric_limits<double>::max();
    for(const auto &entry : entries)
        earliest = std::min(earliest, entry.toObject().value("clip").toObject().value("startTime").toDouble());

    const auto &layers = t_sequence->layers();
    const int targetIndex = layers.indexOf(t_targetLayer);

    for(const auto &value : entries)
    {
        const QJsonObject entry = value.toObject();
        QJsonObject clipJson = entry.value("clip").toObject();
        clipJson.insert("startTime", clipJson.value("startTime").toDouble() - earliest + t_time);

        ClipLayer *destination = t_targetLayer;
        const int index = targetIndex + entry.value("layerOffset").toInt();
        if(targetIndex >= 0 && index < layers.size())
            if(auto *clipLayer = dynamic_cast<ClipLayer *>(layers[index]))
                destination = clipLayer;

        if(Clip *clip = destination->addClipFromJson(clipJson))
            pasted.append(clip);
    }
    return pasted;
}

} // namespace ClipClipboard

} // namespace photon
