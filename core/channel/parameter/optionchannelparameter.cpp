#include "optionchannelparameter.h"
#include "view/optionchannelparameterview.h"

namespace photon {

class OptionChannelParameter::Impl
{
public:
    QStringList options;
    int defaultValue = 0;
};

OptionChannelParameter::OptionChannelParameter() : ChannelParameter(""),m_impl(new Impl)
{

}
OptionChannelParameter::OptionChannelParameter(const QString &name, const QStringList &values, int defaultValue) : ChannelParameter(name),m_impl(new Impl)
{
    m_impl->options = values;
    m_impl->defaultValue = defaultValue;

}
OptionChannelParameter::~OptionChannelParameter()
{
    delete m_impl;
}

QStringList OptionChannelParameter::options() const
{
    return m_impl->options;
}

int OptionChannelParameter::defaultValue() const
{
    return m_impl->defaultValue;
}

ChannelParameterView *OptionChannelParameter::createView()
{
    return new OptionChannelParameterView(this);
}

void OptionChannelParameter::readFromJson(const QJsonObject &t_json)
{
    ChannelParameter::readFromJson(t_json);
    auto optionArray = t_json.value("options").toArray();
    for(auto option : optionArray)
    {
        m_impl->options.append(option.toString());
    }
    m_impl->defaultValue = t_json.value("default").toInt();
}

void OptionChannelParameter::writeToJson(QJsonObject &t_json) const
{
    ChannelParameter::writeToJson(t_json);
    QJsonArray array;

    for(auto option : m_impl->options)
        array.append(option);

    t_json.insert("options", array);
    t_json.insert("default", m_impl->defaultValue);
}

} // namespace photon
