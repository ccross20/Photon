#ifndef PHOTON_SEQUENCE_H
#define PHOTON_SEQUENCE_H

#include "photon-global.h"
#include "processcontext.h"


namespace photon {

class PHOTONCORE_EXPORT Sequence : public QObject
{
    Q_OBJECT
public:
    explicit Sequence(const QString &t_name = QString{}, QObject *parent = nullptr);
    ~Sequence();

    void init();
    QString name() const;
    void setName(const QString &);
    QString filePath() const;
    void setAudioPath(const QString &);
    void addLayer(Layer *);
    void removeLayer(Layer *);
    void addCueLayer(CueLayer *);
    void removeCueLayer(CueLayer *);
    CueLayer *editableCueLayer() const;
    void setEditableCueLayer(CueLayer *);
    const QVector<CueLayer*> &cueLayers() const;
    bool findClosestBeatToTime(float, float *) const;
    bool snapToBeat(float time, float *outTime, float tolerance = .1) const;

    Layer *findLayerByGuid(const QUuid &guid);
    const QVector<Layer*> &layers() const;
    Project *project() const;

    // The sequence's song analysis (beats, level/frequency envelopes). Owned here;
    // persisted to a binary sidecar next to the .seq on save/load.
    SongData *songData() const;

    // Editor playhead position, updated by whichever SequenceWidget has this
    // sequence open (or by a live preview driver) and read by anything wanting to
    // preview the sequence at "wherever the timeline is right now" - e.g.
    // SequenceNode. Thread-safe: safe to call from any thread.
    double previewTime() const;
    void setPreviewTime(double);

    void processChannels(ProcessContext &, double lastTime);


    void save(const QString &path = QString{}) const;
    void load(const QString &path = QString{});
    void restore(Project &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    void layerUpdated(photon::Layer *);
    void layerAdded(photon::Layer *);
    void layerRemoved(photon::Layer *);
    void fileChanged(const QString &);
    void cueLayerAdded(photon::CueLayer *);
    void cueLayerRemoved(photon::CueLayer *);
    void editableCueLayerChanged(photon::CueLayer *);

private slots:

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SEQUENCE_H
