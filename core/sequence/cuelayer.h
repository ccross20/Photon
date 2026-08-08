#ifndef PHOTON_CUELAYER_H
#define PHOTON_CUELAYER_H

#include <QObject>
#include <QColor>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT CueLayer : public QObject
{
    Q_OBJECT
public:
    explicit CueLayer(QObject *parent = nullptr);
    ~CueLayer();

    void setName(const QString &);
    QString name() const;
    void addMarkers(const QList<float> &);
    void replaceMarkers(const QList<float> &);
    void insertMarker(float);
    void setIsVisible(bool);
    bool isVisible() const;
    void setIsSnappable(bool);
    bool isSnappable() const;
    void setIsEditable(bool);
    bool isEditable() const;
    void setColor(const QColor &);
    QColor color() const;

    bool snapToMarker(float time, float *outTime, float tolerance = .1) const;
    const QList<float> &markers() const;

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

public slots:
    void sort();

signals:
    void markersChanged(photon::CueLayer *);
    void metadataChanged(photon::CueLayer *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CUELAYER_H
