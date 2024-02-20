#include "fileparameter.h"
#include "view/filechannelparameterview.h"

namespace photon {

class FileParameter::Impl
{
public:
    QString defaultValue;
};

FileParameter::FileParameter(): ChannelParameter(""),m_impl(new Impl)
{

}

FileParameter::FileParameter(const QString &t_name, const QString &t_default) : ChannelParameter(t_name,ChannelParameterText,1),m_impl(new Impl)
{
    m_impl->defaultValue = t_default;
    setValue(t_default);
}

FileParameter::~FileParameter()
{
    delete m_impl;
}

QString FileParameter::defaultValue() const
{
    return m_impl->defaultValue;
}

ChannelParameterView *FileParameter::createView()
{
    return new FileChannelParameterView(this);
}

void FileParameter::readFromJson(const QJsonObject &t_json)
{
    ChannelParameter::readFromJson(t_json);
    m_impl->defaultValue = t_json.value("default").toString();
    setValue(t_json.value("value").toString());
}

void FileParameter::writeToJson(QJsonObject &t_json) const
{
    ChannelParameter::writeToJson(t_json);
    t_json.insert("default", m_impl->defaultValue);
    t_json.insert("value", value().toString());
}

} // namespace photon
