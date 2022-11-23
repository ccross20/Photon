#ifndef PHOTON_SEQUENCE_H
#define PHOTON_SEQUENCE_H

#include "photon-global.h"
#include "data/dmxmatrix.h"

namespace photon {

class ProcessContext
{
public:
    ProcessContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    Fixture *fixture = nullptr;
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
    void addLayer(Layer *);
    void removeLayer(Layer *);
    const QVector<Layer*> &layers() const;
    Project *project() const;

    void processChannels(ProcessContext &, double lastTime);

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    void layerUpdated(photon::Layer *);
    void layerAdded(photon::Layer *);
    void layerRemoved(photon::Layer *);

private slots:

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SEQUENCE_H
