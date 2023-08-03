#include "numberchannelparameter.h"
#include "view/numberchannelparameterview.h"

namespace photon {

class NumberChannelParameter::Impl
{
public:
    double min = std::numeric_limits<double>::lowest();
    double max = std::numeric_limits<double>::max();
    double increment = .01;
    double defaultValue = 0.0;
};

NumberChannelParameter::NumberChannelParameter() : ChannelParameter(""),m_impl(new Impl)
{

}

NumberChannelParameter::NumberChannelParameter(const QString &t_name,double t_default, double t_min, double t_max, double t_increment) : ChannelParameter(t_name),m_impl(new Impl)
{
    m_impl->min = t_min;
    m_impl->max = t_max;
    m_impl->increment = t_increment;
    m_impl->defaultValue = t_default;
    setValue(t_default);
}

NumberChannelParameter::~NumberChannelParameter()
{
    delete m_impl;
}

double NumberChannelParameter::increment() const
{
    return m_impl->increment;
}

double NumberChannelParameter::minimum() const
{
    return m_impl->min;
}

double NumberChannelParameter::maximum() const
{
    return m_impl->max;
}

double NumberChannelParameter::defaultValue() const
{
    return m_impl->defaultValue;
}

void NumberChannelParameter::setRange(double t_min, double t_max, double t_increment)
{
    m_impl->min = t_min;
    m_impl->max = t_max;
    m_impl->increment = t_increment;
}

void NumberChannelParameter::setIncrement(double t_increment)
{
    m_impl->increment = t_increment;
}

void NumberChannelParameter::setMinimum(double t_min)
{
    m_impl->min = t_min;
}

void NumberChannelParameter::setMaximum(double t_max)
{
    m_impl->max = t_max;
}

ChannelParameterView *NumberChannelParameter::createView()
{
    return new NumberChannelParameterView(this);
}

void NumberChannelParameter::readFromJson(const QJsonObject &t_json)
{
    m_impl->min = t_json.value("min").toDouble();
    m_impl->max = t_json.value("max").toDouble();
    m_impl->increment = t_json.value("increment").toDouble();
    m_impl->defaultValue = t_json.value("default").toDouble();
    setValue(t_json.value("value").toDouble());
}

void NumberChannelParameter::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("min", m_impl->min);
    t_json.insert("max", m_impl->max);
    t_json.insert("increment", m_impl->increment);
    t_json.insert("default", m_impl->defaultValue);
    t_json.insert("value", value().toDouble());
}

} // namespace photon
