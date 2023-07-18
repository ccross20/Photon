#ifndef PHOTON_PIXELSOURCE_H
#define PHOTON_PIXELSOURCE_H

#include <QImage>
#include "photon-global.h"
#include "canvas.h"
#include "fixture/dmxreceiver.h"

namespace photon {


class PHOTONCORE_EXPORT PixelSource : public DMXReceiver
{
public:

    enum PixelFormat{
        Format_RGB,
        Format_RGBW
    };

    PixelSource();
    virtual ~PixelSource();

    void setFormat(PixelFormat);
    PixelFormat format() const;
    virtual const QVector<QPointF> &positions() const = 0;
    QVector3D boundingVector() const;

    virtual int dmxOffset() const override;
    virtual int dmxSize() const override;
    virtual int universe() const override;

    virtual void process(CanvasContext &, const QTransform &) const;

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;
};

} // namespace photon

#endif // PHOTON_PIXELSOURCE_H
