#ifndef PHOTON_SEQUENCE_H
#define PHOTON_SEQUENCE_H

#include "photon-global.h"
#include "processcontext.h"
#include "project/projectresource.h"


namespace photon {

class PHOTONCORE_EXPORT Sequence : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    explicit Sequence(const QString &t_name = QString{}, QObject *parent = nullptr);
    ~Sequence();

    void init();
    QString name() const;
    void setName(const QString &);
    QByteArray uniqueId() const;
    QString filePath() const;

    // Whether this sequence is owned by the Song Library (its own .seq file
    // on disk) rather than embedded directly in the current project's JSON.
    // Set once by whoever creates/loads it - see Project::readFromJson vs.
    // PhotonCore::loadSequence()/SongLibraryPanel.
    bool isLibrarySequence() const;
    void setIsLibrarySequence(bool);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "sequence";}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override;
    void setAudioPath(const QString &);
    void addLayer(Layer *);
    void removeLayer(Layer *);
    void addCueLayer(CueLayer *);
    void removeCueLayer(CueLayer *);
    CueLayer *editableCueLayer() const;
    void setEditableCueLayer(CueLayer *);
    const QVector<CueLayer*> &cueLayers() const;
    // Nearest snap point for a clip being dragged/resized to `time`: any
    // snap-enabled cue layer's markers, or the start/end of any other clip
    // in the sequence. excludeClips keeps the clip(s) currently being
    // dragged from snapping to themselves.
    bool snapTime(float time, float *outTime, float tolerance = .1, const QVector<Clip*> &excludeClips = {}) const;

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
