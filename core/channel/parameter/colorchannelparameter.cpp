#include "colorchannelparameter.h"
#include "view/colorchannelparameterview.h"
#include "util/utils.h"

namespace photon {


class ColorChannelParameter::Impl
{
public:
    QColor defaultValue;
};

ColorChannelParameter::ColorChannelParameter() : ChannelParameter(""),m_impl(new Impl)
{

}


ColorChannelParameter::ColorChannelParameter(const QString &t_name,QColor t_default) : ChannelParameter(t_name, ChannelParameterColor,4),m_impl(new Impl)
{
    m_impl->defaultValue = t_default;
    setValue(t_default);
    setChannelNames({"Hue","Saturation","Lightness","Alpha"});
}

ColorChannelParameter::~ColorChannelParameter()
{
    delete m_impl;
}

ChannelInfo ColorChannelParameter::channelInfo() const
{
    ChannelInfo info = ChannelParameter::channelInfo();
    info.type = ChannelInfo::ChannelTypeColor;
    return info;
}

QColor ColorChannelParameter::defaultValue() const
{
    return m_impl->defaultValue;
}


QColor ColorChannelParameter::channelsToColor(const float* t_channels)
{
    return QColor::fromHslF(t_channels[0], t_channels[1], t_channels[2], t_channels[3]);
}

void ColorChannelParameter::colorToChannels(const QColor &t_value, float* values)
{
    float h,l,s,a;
    t_value.getHslF(&h, &s, &l, &a);
    values[0] = h;
    values[1] = s;
    values[2] = l;
    values[3] = a;

}

QVariant ColorChannelParameter::channelsToVariant(const QVector<double> &t_channels) const
{
    return QColor::fromHslF(t_channels[0], t_channels[1], t_channels[2], t_channels[3]);
}

QVector<double> ColorChannelParameter::variantToChannels(const QVariant &t_value) const
{
    QColor color = t_value.value<QColor>();
    float h,l,s,a;
    color.getHslF(&h, &s, &l, &a);
    return QVector<double>{h,s,l,a};
}


ChannelParameterView *ColorChannelParameter::createView()
{
    return new ColorChannelParameterView(this);
}


void ColorChannelParameter::readFromJson(const QJsonObject &t_json)
{
    ChannelParameter::readFromJson(t_json);
    m_impl->defaultValue = jsonToColor(t_json.value("default").toObject());
    setValue(jsonToColor(t_json.value("value").toObject()));
}

void ColorChannelParameter::writeToJson(QJsonObject &t_json) const
{
    ChannelParameter::writeToJson(t_json);
    t_json.insert("default", colorToJson(m_impl->defaultValue));
    t_json.insert("value", colorToJson(value().value<QColor>()));
}



} // namespace photon
