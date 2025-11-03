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
    QVector<FixtureCapability *>capabilities;
    QVector<QPointF> positions;
    QByteArray uniqueId;
};

FixturePixelSource::FixturePixelSource(const QVector<FixtureCapability *> &t_capabilities) : PixelSource(),m_impl(new Impl) {
    m_impl->capabilities = t_capabilities;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();

    double length = .9;
    int pixelCount = t_capabilities.length();
    double delta = length / (pixelCount - 1);
    double center = .5;
    double angle = 0;

    QTransform t;
    t.translate(length * center, 0);
    t.rotate(angle);
    t.translate(length * -center, 0);

    m_impl->positions.resize(pixelCount);
    double position = 0;
    for(auto it = m_impl->positions.begin(); it != m_impl->positions.end(); ++it)
    {
        auto &pt = *it;
        auto transformedPt = t.map(QPointF(position, 0.0));

        pt.setX(transformedPt.x());
        pt.setY(transformedPt.y());
        position += delta;
    }
}

FixturePixelSource::~FixturePixelSource()
{
    delete m_impl;
}

const QVector<FixtureCapability *> &FixturePixelSource::capabilities() const
{
    return m_impl->capabilities;
}

Fixture *FixturePixelSource::fixture() const
{
    if(m_impl->capabilities.isEmpty())
        return nullptr;
    return m_impl->capabilities[0]->fixture();
}

int FixturePixelSource::dmxOffset() const
{
    if(m_impl->capabilities.isEmpty())
        return 0;
    return m_impl->capabilities[0]->fixture()->dmxOffset() + m_impl->capabilities[0]->range().start;
}

int FixturePixelSource::dmxSize() const
{
    return 3;
}

int FixturePixelSource::universe() const
{
    return m_impl->capabilities[0]->fixture()->universe();
}

const QVector<QPointF> &FixturePixelSource::positions() const
{
    return m_impl->positions;
}

void FixturePixelSource::process(ProcessContext &t_context, const QTransform &t_transform) const
{
    auto capIt = m_impl->capabilities.cbegin();


        if(!t_context.image)
            return;

        for(auto it = positions().cbegin(); it != positions().cend() && capIt != m_impl->capabilities.cend(); it++, capIt++)
        {

            ColorCapability *colorCap = dynamic_cast<ColorCapability*>(*capIt);
            if(colorCap)
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
            }


            /*

        auto rgbw = m_impl->GetRgbwFromRgb(tRGB{qc.red(), qc.green(), qc.blue()},2700);

        t_context.dmxMatrix.setValue(u, channel++, rgbw.r);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.g);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.b);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.w);
*/
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
    return m_impl->capabilities[0]->fixture()->uniqueId();
}


} // namespace photon
