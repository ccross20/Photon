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

    // How many pixel slots this source has - NOT where they sit. Spatial
    // placement is the owning PixelSourceLayout's concern, not the source's;
    // positions are supplied to process()/collectSampleUVs() by the caller.
    virtual int pixelCount() const = 0;
    QVector3D boundingVector() const;

    virtual QByteArray sourceUniqueId() const = 0;
    virtual int dmxOffset() const override;
    virtual int dmxSize() const override;
    virtual int universe() const override;

    // t_positions are already in final canvas/UV space - the caller (a
    // PixelSourceLayout) owns placement, there's no further transform to apply.
    virtual void process(ProcessContext &, const QVector<QPointF> &positions, double blend = 1.0) const;

    // Appends t_positions, in the same order process() consumes colours.
    // Used to build the GPU gather list (5b).
    virtual void collectSampleUVs(QVector<QPointF> &out, const QVector<QPointF> &positions) const;

    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELSOURCE_H
