#ifndef PHOTON_BEATLAYER_H
#define PHOTON_BEATLAYER_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT BeatLayer : public QObject
{
    Q_OBJECT
public:
    explicit BeatLayer(QObject *parent = nullptr);
    ~BeatLayer();

    void setName(const QString &);
    QString name() const;
    void addBeats(const QList<float> &);
    void replaceBeats(const QList<float> &);
    void insertBeat(float);
    void setIsVisible(bool);
    bool isVisible() const;
    void setIsSnappable(bool);
    bool isSnappable() const;
    void setIsEditable(bool);
    bool isEditable() const;

    bool snapToBeat(float time, float *outTime, float tolerance = .1) const;
    const QList<float> &beats() const;

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

public slots:
    void sort();

signals:
    void beatsChanged(photon::BeatLayer *);
    void metadataChanged(photon::BeatLayer *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BEATLAYER_H
