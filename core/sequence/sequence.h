#ifndef PHOTON_SEQUENCE_H
#define PHOTON_SEQUENCE_H

#include "photon-global.h"
#include "data/dmxmatrix.h"

class QOpenGLContext;

namespace photon {

class OpenGLFrameBuffer;

class ProcessContext
{
public:
    ProcessContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    Fixture *fixture = nullptr;
    Project *project = nullptr;
    QImage *canvasImage = nullptr;
    QImage *previousCanvasImage = nullptr;
    OpenGLFrameBuffer *frameBuffer = nullptr;
    Canvas *canvas = nullptr;
    QOpenGLContext *openglContext;
    DMXMatrix &dmxMatrix;
    double globalTime;
    double relativeTime;
};

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
    void addBeatLayer(BeatLayer *);
    void removeBeatLayer(BeatLayer *);
    BeatLayer *editableBeatLayer() const;
    void setEditableBeatLayer(BeatLayer *);
    const QVector<BeatLayer*> &beatLayers() const;
    bool findClosestBeatToTime(float, float *) const;
    bool snapToBeat(float time, float *outTime, float tolerance = .1) const;

    Layer *findLayerByGuid(const QUuid &guid);
    const QVector<Layer*> &layers() const;
    Project *project() const;

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
    void beatLayerAdded(photon::BeatLayer *);
    void beatLayerRemoved(photon::BeatLayer *);
    void editableBeatLayerChanged(photon::BeatLayer *);

private slots:

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SEQUENCE_H
