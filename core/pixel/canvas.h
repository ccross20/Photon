#ifndef PHOTON_CANVAS_H
#define PHOTON_CANVAS_H

#include <QObject>
#include <QImage>
#include "data/dmxmatrix.h"
#include "photon-global.h"

namespace photon {

struct CanvasContext
{
    QImage pixelData;
    DMXMatrix dmxMatrix;
};

class PHOTONCORE_EXPORT Canvas : public QObject
{
    Q_OBJECT
public:
    explicit Canvas(QObject *parent = nullptr);
    Canvas(const QString &name,  const QSize &size, QObject *parent = nullptr);
    ~Canvas();

    QSize size() const;
    void setSize(const QSize &);

    int width() const;
    int height() const;

    QString name() const;
    void setName(const QString &);

    void paint(const QImage &);

    OpenGLTexture *texture() const;
    void updateTexture();

    void restore(Project &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    void metadataUpdated();
    void textureDidUpdate();

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_CANVAS_H
