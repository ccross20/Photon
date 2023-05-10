#include <QColor>
#include "colorcapability.h"

namespace photon {


class ColorCapability::Impl
{
public:
    QVector<ColorIntensityCapability*> channels;
};


ColorCapability::ColorCapability(const QVector<ColorIntensityCapability*> &t_channels) : FixtureCapability(DMXRange(),Capability_Color),m_impl(new Impl)
{
    m_impl->channels = t_channels;
}

ColorCapability::~ColorCapability()
{
    delete m_impl;
}

void ColorCapability::setColor(const QColor &t_color, DMXMatrix &t_matrix, double t_blend) const
{
    QColor c = t_color.toCmyk();
    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Cyan:
            channel->setPercent(c.cyanF(), t_matrix, t_blend);
            break;
        case Capability_Magenta:
            channel->setPercent(c.magentaF(), t_matrix, t_blend);
            break;
        case Capability_Yellow:
            channel->setPercent(c.yellowF(), t_matrix, t_blend);
            break;
        default:
            break;
        }

    }

}

QColor ColorCapability::getColor(const DMXMatrix &t_matrix) const
{
    QColor color;
    float c = 1.0f,m = 1.0f,y= 1.0f, k = 0.0f;

    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Cyan:
            c = channel->getPercent(t_matrix);
            break;
        case Capability_Magenta:
            m = channel->getPercent(t_matrix);
            break;
        case Capability_Yellow:
            y = channel->getPercent(t_matrix);
            break;
        default:
            break;
        }

    }
    color.setCmykF(c,m,y,k);
    return color;
}

} // namespace photon
