#include "colorchannelparameter.h"
#include "view/colorchannelparameterview.h"

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

QColor ColorChannelParameter::defaultValue() const
{
    return m_impl->defaultValue;
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

QColor jsonToColor(const QJsonObject &t_json)
{
    return QColor::fromHslF(t_json.value("h").toDouble(), t_json.value("s").toDouble(),
                            t_json.value("l").toDouble(),t_json.value("a").toDouble());
}

QJsonObject jsonFromColor(const QColor &t_color)
{
    float h,l,s,a;
    t_color.getHslF(&h, &l, &s, &a);

    QJsonObject obj;
    obj.insert("h", h);
    obj.insert("s", s);
    obj.insert("l", l);
    obj.insert("a", a);

    return obj;
}

void ColorChannelParameter::readFromJson(const QJsonObject &t_json)
{
    m_impl->defaultValue = jsonToColor(t_json.value("default").toObject());
    setValue(jsonToColor(t_json.value("value").toObject()));
}

void ColorChannelParameter::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("default", jsonFromColor(m_impl->defaultValue));
    t_json.insert("value", jsonFromColor(value().value<QColor>()));
}



} // namespace photon
