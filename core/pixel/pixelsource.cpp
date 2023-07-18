#include "pixelsource.h"

namespace photon {

PixelSource::PixelSource()
{

}

PixelSource::~PixelSource()
{

}

void PixelSource::setFormat(PixelFormat)
{

}

PixelSource::PixelFormat PixelSource::format() const
{
    return Format_RGBW;
}


QVector3D PixelSource::boundingVector() const
{
    return QVector3D{};
}

int PixelSource::dmxOffset() const
{
    return 0;
}

int PixelSource::dmxSize() const
{
    return 1;
}

int PixelSource::universe() const
{
    return 1;
}

void PixelSource::process(CanvasContext &t_context, const QTransform &t_transform) const
{
    int u = universe();
    int channel = dmxOffset();
    for(auto it = positions().cbegin(); it != positions().cend(); it++)
    {
        auto pt = t_transform.map((*it).toPoint());
        pt.setX(pt.x() * t_context.pixelData.width());
        pt.setY(pt.y() * t_context.pixelData.height());
        auto color = t_context.pixelData.pixel(pt);

        auto qc = QColor::fromRgb(color);

        t_context.dmxMatrix.setValue(u, channel++, qc.red());
        t_context.dmxMatrix.setValue(u, channel++, qc.green());
        t_context.dmxMatrix.setValue(u, channel++, qc.blue());
        channel++; // white
    }
}

void PixelSource::readFromJson(const QJsonObject &)
{

}

void PixelSource::writeToJson(QJsonObject &) const
{

}

} // namespace photon
