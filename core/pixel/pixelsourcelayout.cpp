#include "pixelsourcelayout.h"
#include "pixelsource.h"

namespace photon {

class PixelSourceLayout::Impl
{
public:
    QPointF position;
    QPointF scale;
    double rotation;
    QString name;
    QByteArray uniqueId;
    PixelSource *source;
};

PixelSourceLayout::PixelSourceLayout(PixelSource *t_source) : QObject(), m_impl(new Impl)
{
    m_impl->source = t_source;
}

PixelSourceLayout::~PixelSourceLayout()
{

}

void PixelSourceLayout::setPosition(const QPointF &)
{
    delete m_impl;
}

QPointF PixelSourceLayout::position() const
{
    return m_impl->position;
}


void PixelSourceLayout::setRotation(double)
{

}

double PixelSourceLayout::rotation() const
{
    return m_impl->rotation;
}


void PixelSourceLayout::setScale(const QPointF &)
{

}

QPointF PixelSourceLayout::scale() const
{
    return m_impl->scale;
}


QTransform PixelSourceLayout::transform() const
{
    QTransform tform;

    return tform;
}


QRectF PixelSourceLayout::canvasBounds() const
{
    return QRectF{};
}


QRectF PixelSourceLayout::localBounds() const
{
    return QRectF{};
}

PixelSource *PixelSourceLayout::source() const
{
    return m_impl->source;
}

void PixelSourceLayout::setSource(PixelSource *t_source)
{
    m_impl->source = t_source;
}

void PixelSourceLayout::process(CanvasContext &t_context) const
{
    m_impl->source->process(t_context, transform());
}

void PixelSourceLayout::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{

}

void PixelSourceLayout::writeToJson(QJsonObject &t_json) const
{

}

} // namespace photon
