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
    QByteArray uniqueId;
};

FixturePixelSource::FixturePixelSource(const QVector<FixtureCapability *> &t_capabilities) : PixelSource(),m_impl(new Impl) {
    m_impl->capabilities = t_capabilities;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
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

int FixturePixelSource::pixelCount() const
{
    // The total capability count, not just the color-emitting ones - the
    // caller-supplied positions vector is index-parallel to capabilities()
    // and both collectSampleUVs()/process() lockstep-iterate the two, only
    // acting on ColorCapability slots while still consuming a position slot
    // for every capability. Counting color capabilities only would desync
    // the two arrays.
    return m_impl->capabilities.size();
}

void FixturePixelSource::collectSampleUVs(QVector<QPointF> &t_out, const QVector<QPointF> &t_positions) const
{
    auto capIt = m_impl->capabilities.cbegin();
    for(auto it = t_positions.cbegin(); it != t_positions.cend() && capIt != m_impl->capabilities.cend(); ++it, ++capIt)
    {
        if(dynamic_cast<ColorCapability*>(*capIt))
            t_out << *it;
    }
}

void FixturePixelSource::process(ProcessContext &t_context, const QVector<QPointF> &t_positions, double t_blend) const
{
    auto capIt = m_impl->capabilities.cbegin();


        if(!t_context.gatheredColors && !t_context.image)
            return;

        for(auto it = t_positions.cbegin(); it != t_positions.cend() && capIt != m_impl->capabilities.cend(); it++, capIt++)
        {

            ColorCapability *colorCap = dynamic_cast<ColorCapability*>(*capIt);
            if(colorCap)
            {
                QColor qc;
                if(t_context.gatheredColors)
                {
                    qc = t_context.gatheredIndex < t_context.gatheredColors->size()
                             ? t_context.gatheredColors->at(t_context.gatheredIndex++) : QColor(0,0,0);
                }
                else
                {
                    auto ptF = (*it);
                    ptF.setX(ptF.x() * t_context.image->width());
                    ptF.setY(ptF.y() * t_context.image->height());

                    auto pt = ptF.toPoint();
                    QRgb color;
                    if(pt.x() < 0 || pt.x() >= t_context.image->width() || pt.y() < 0 || pt.y() >= t_context.image->height())
                        color = 0;
                    else
                        color = t_context.image->pixel(pt);

                    qc = QColor::fromRgb(color);
                }

                colorCap->setColor(qc, t_context.dmxMatrix,t_blend);
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
