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
    setChannel(t_channels[0]->channel());
}

ColorCapability::~ColorCapability()
{
    delete m_impl;
}

void ColorCapability::setColor(const QColor &t_color, DMXMatrix &t_matrix, double t_blend) const
{
    /*
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
*/

    QColor c = t_color.toRgb();
    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Red:
            channel->setPercent(c.redF(), t_matrix, t_blend);
            break;
        case Capability_Green:
            channel->setPercent(c.greenF(), t_matrix, t_blend);
            break;
        case Capability_Blue:
            channel->setPercent(c.blueF(), t_matrix, t_blend);
            break;
        case Capability_Lime:
            //channel->setPercent(c.blueF(), t_matrix, t_blend);
            break;
        case Capability_Amber:
        {
            float amber = (((c.redF()*255.0f)*1.0f+(c.greenF()*255.0f)*.749f)/446.0f);

            channel->setPercent(amber, t_matrix, t_blend);
        }
            break;
        case Capability_UV:
            channel->setPercent(c.blueF(), t_matrix, t_blend);
            break;
        default:
            break;
        }

    }

}

QColor ColorCapability::getColor(const DMXMatrix &t_matrix) const
{
    /*
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
*/

    QColor color;
    float r = 1.0f,g = 1.0f,b= 1.0f;

    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Red:
            r = channel->getPercent(t_matrix);
            break;
        case Capability_Green:
            g = channel->getPercent(t_matrix);
            break;
        case Capability_Blue:
            b = channel->getPercent(t_matrix);
            break;
        default:
            break;
        }

    }
    color.setRgbF(r,g,b);
    return color;
}

} // namespace photon
