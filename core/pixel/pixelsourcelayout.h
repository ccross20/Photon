#ifndef PHOTON_PIXELSOURCELAYOUT_H
#define PHOTON_PIXELSOURCELAYOUT_H

#include "photon-global.h"
#include "pixel/canvas.h"

namespace photon {

class PHOTONCORE_EXPORT PixelSourceLayout : public QObject
{
    Q_OBJECT
public:
    PixelSourceLayout(PixelSource *source = nullptr);
    ~PixelSourceLayout();

    void setPosition(const QPointF &);
    QPointF position() const;

    void setRotation(double);
    double rotation() const;

    void setScale(const QPointF &);
    QPointF scale() const;

    QTransform transform() const;

    bounds_d localBounds() const;
    bounds_d canvasBounds() const;

    PixelSource *source() const;
    void setSource(PixelSource *);

    void process(ProcessContext &) const;

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    void transformUpdated();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELSOURCELAYOUT_H
