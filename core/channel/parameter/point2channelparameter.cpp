#include "point2channelparameter.h"
#include "view/point2channelparameterview.h"

namespace photon {

class Point2ChannelParameter::Impl
{
public:
    QPointF min = QPointF{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    QPointF max = QPointF{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    double increment = .01;
    QPointF defaultValue;
};

Point2ChannelParameter::Point2ChannelParameter() : ChannelParameter(""),m_impl(new Impl)
{

}

Point2ChannelParameter::Point2ChannelParameter(const QString &t_name,QPointF t_default, QPointF t_min, QPointF t_max, double t_increment) : ChannelParameter(t_name, ChannelParameterPoint,2),m_impl(new Impl)
{
    m_impl->min = t_min;
    m_impl->max = t_max;
    m_impl->increment = t_increment;
    m_impl->defaultValue = t_default;
    setValue(t_default);
    setChannelNames({"X","Y"});
}

Point2ChannelParameter::~Point2ChannelParameter()
{
    delete m_impl;
}

QVariant Point2ChannelParameter::channelsToVariant(const QVector<double> &t_channels) const
{
    return QPointF{t_channels[0], t_channels[1]};
}

QVector<double> Point2ChannelParameter::variantToChannels(const QVariant &t_value) const
{
    QPointF pt = t_value.value<QPointF>();
    return QVector<double>{pt.x(), pt.y()};
}

double Point2ChannelParameter::increment() const
{
    return m_impl->increment;
}

QPointF Point2ChannelParameter::minimum() const
{
    return m_impl->min;
}

QPointF Point2ChannelParameter::maximum() const
{
    return m_impl->max;
}

void Point2ChannelParameter::setRange(QPointF t_min, QPointF t_max, double t_increment)
{
    m_impl->min = t_min;
    m_impl->max = t_max;
    m_impl->increment = t_increment;
}

void Point2ChannelParameter::setIncrement(double t_increment)
{
    m_impl->increment = t_increment;
}

void Point2ChannelParameter::setMinimum(QPointF t_min)
{
    m_impl->min = t_min;
}

void Point2ChannelParameter::setMaximum(QPointF t_max)
{
    m_impl->max = t_max;
}

ChannelParameterView *Point2ChannelParameter::createView()
{
    return new Point2ChannelParameterView(this);
}

QPointF jsonToPoint(const QJsonObject &t_json)
{
    return QPointF{t_json.value("x").toDouble(), t_json.value("y").toDouble()};
}

QJsonObject jsonFromPoint(const QPointF &t_pt)
{
    QJsonObject obj;
    obj.insert("x", t_pt.x());
    obj.insert("y", t_pt.y());

    return obj;
}

void Point2ChannelParameter::readFromJson(const QJsonObject &t_json)
{
    m_impl->min = jsonToPoint(t_json.value("min").toObject());
    m_impl->max = jsonToPoint(t_json.value("max").toObject());
    m_impl->increment = t_json.value("increment").toDouble();
    setValue(jsonToPoint(t_json.value("value").toObject()));
}

void Point2ChannelParameter::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("min", jsonFromPoint(m_impl->min));
    t_json.insert("max", jsonFromPoint(m_impl->max));
    t_json.insert("increment", m_impl->increment);
    t_json.insert("value", jsonFromPoint(value().value<QPointF>()));
}

} // namespace photon
