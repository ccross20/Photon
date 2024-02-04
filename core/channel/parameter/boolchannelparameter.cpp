#include "boolchannelparameter.h"
#include "view/boolchannelparameterview.h"

namespace photon {

class BoolChannelParameter::Impl
{
public:
    bool defaultValue = true;
};

BoolChannelParameter::BoolChannelParameter(): ChannelParameter(""),m_impl(new Impl)
{

}

BoolChannelParameter::BoolChannelParameter(const QString &t_name, bool t_default) : ChannelParameter(t_name),m_impl(new Impl)
{
    m_impl->defaultValue = t_default;
    setValue(t_default);
}

BoolChannelParameter::~BoolChannelParameter()
{
    delete m_impl;
}

bool BoolChannelParameter::defaultValue() const
{
    return m_impl->defaultValue;
}

ChannelParameterView *BoolChannelParameter::createView()
{
    return new BoolChannelParameterView(this);
}

void BoolChannelParameter::readFromJson(const QJsonObject &t_json)
{
    ChannelParameter::readFromJson(t_json);
    m_impl->defaultValue = t_json.value("default").toDouble();
    setValue(t_json.value("value").toDouble());
}

void BoolChannelParameter::writeToJson(QJsonObject &t_json) const
{
    ChannelParameter::writeToJson(t_json);
    t_json.insert("default", m_impl->defaultValue);
    t_json.insert("value", value().toDouble());
}

} // namespace photon
