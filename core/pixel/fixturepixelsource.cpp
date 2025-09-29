#include <QImage>
#include "fixturepixelsource.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/fixture.h"
#include "opengl/openglframebuffer.h"
#include "pixel/canvas.h"
#include "sequence/sequence.h"

namespace photon {

class FixturePixelSource::Impl
{
public:
    FixtureCapability *capability;
    QVector<QPointF> positions;
    QByteArray uniqueId;
};

FixturePixelSource::FixturePixelSource(FixtureCapability *t_capability) : PixelSource(),m_impl(new Impl) {
    m_impl->capability = t_capability;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->positions.append(QPointF());
}

FixturePixelSource::~FixturePixelSource()
{
    delete m_impl;
}

FixtureCapability *FixturePixelSource::capability() const
{
    return m_impl->capability;
}

Fixture *FixturePixelSource::fixture() const
{
    return m_impl->capability->fixture();
}

int FixturePixelSource::dmxOffset() const
{
    return m_impl->capability->fixture()->dmxOffset() + m_impl->capability->range().start;
}

int FixturePixelSource::dmxSize() const
{
    return 3;
}

int FixturePixelSource::universe() const
{
    return m_impl->capability->fixture()->universe();
}

const QVector<QPointF> &FixturePixelSource::positions() const
{
    return m_impl->positions;
}

void FixturePixelSource::process(ProcessContext &t_context, const QTransform &t_transform) const
{
    ColorCapability *colorCap = dynamic_cast<ColorCapability*>(m_impl->capability);
    if(colorCap)
    {
        if(!t_context.image)
            return;

        for(auto it = positions().cbegin(); it != positions().cend(); it++)
        {
            auto ptF = t_transform.map((*it));
            ptF.setX(ptF.x() * t_context.canvas->width());
            ptF.setY(ptF.y() * t_context.canvas->height());

            auto pt = ptF.toPoint();
            QRgb color;
            if(pt.x() < 0 || pt.x() >= t_context.canvas->width() || pt.y() < 0 || pt.y() >= t_context.canvas->height())
            {
                color = 0;
            }
            else
            {
                color = t_context.image->pixel(pt);
            }

            auto qc = QColor::fromRgb(color);

            colorCap->setColor(qc, t_context.dmxMatrix);

            /*

        auto rgbw = m_impl->GetRgbwFromRgb(tRGB{qc.red(), qc.green(), qc.blue()},2700);

        t_context.dmxMatrix.setValue(u, channel++, rgbw.r);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.g);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.b);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.w);
*/
        }
    }


    //PixelSource::process(t_context, t_transform);
}


void FixturePixelSource::readFromJson(const QJsonObject &, const LoadContext &)
{

}

void FixturePixelSource::writeToJson(QJsonObject &) const
{

}


QByteArray FixturePixelSource::sourceUniqueId() const
{
    return m_impl->capability->fixture()->uniqueId();
}


} // namespace photon
