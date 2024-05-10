#include "channelparameter.h"

namespace photon {

class ChannelParameter::Impl
{
public:
    QStringList channelNames;
    QByteArray uniqueId;
    QVariant value;
    QString name;
    int channelCount;
    ChannelParameterType type;
    bool isReadOnly = false;
};

ChannelParameter::ChannelParameter(const QString &t_name,ChannelParameterType t_type, int t_channelCount) : m_impl(new Impl)
{
    m_impl->uniqueId = t_name.toLatin1();
    m_impl->name = t_name;
    m_impl->type = t_type;
    m_impl->channelCount = t_channelCount;
}

ChannelParameter::~ChannelParameter()
{
    delete m_impl;
}

ChannelInfo ChannelParameter::channelInfo() const
{
    return ChannelInfo{m_impl->uniqueId,ChannelInfo::ChannelTypeNumber,m_impl->name};
}

QString ChannelParameter::name() const
{
    return m_impl->name;
}

QStringList ChannelParameter::channelNames() const
{
    return m_impl->channelNames;
}

void ChannelParameter::setChannelNames(const QStringList &t_channelNames)
{
    m_impl->channelNames = t_channelNames;
}

QByteArray ChannelParameter::uniqueId() const
{
    return m_impl->uniqueId;
}

int ChannelParameter::channels()
{
    return m_impl->channelCount;
}

ChannelParameter::ChannelParameterType ChannelParameter::type() const
{
    return m_impl->type;
}

QVariant ChannelParameter::value() const
{
    return m_impl->value;
}

void ChannelParameter::setValue(const QVariant &t_value)
{
    m_impl->value = t_value;
}

QVariant ChannelParameter::channelsToVariant(const QVector<double> &t_values) const
{
    return t_values[0];
}

QVector<double> ChannelParameter::variantToChannels(const QVariant &t_value) const
{
    return QVector<double>{t_value.toDouble()};
}

bool ChannelParameter::isReadOnly() const
{
    return m_impl->isReadOnly;
}

void ChannelParameter::setIsReadOnly(bool t_value)
{
    m_impl->isReadOnly = t_value;
}

void ChannelParameter::readFromJson(const QJsonObject &t_json)
{
    m_impl->channelCount = t_json.value("count").toInt();

    m_impl->channelNames.clear();
    auto nameArray = t_json.value("channelNames").toArray();
    for(auto name : nameArray)
        m_impl->channelNames << name.toString();
    m_impl->name = t_json.value("name").toString();
    m_impl->type = static_cast<ChannelParameterType>(t_json.value("type").toInt());
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    m_impl->isReadOnly = t_json.value("isReadOnly").toBool();
}

void ChannelParameter::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("count", m_impl->channelCount);

    QJsonArray nameArray;
    for(const auto &name : m_impl->channelNames)
        nameArray.append(name);

    t_json.insert("channelNames", nameArray);
    t_json.insert("name", m_impl->name);
    t_json.insert("type", m_impl->type);
    t_json.insert("uniqueId", QString::fromLatin1(m_impl->uniqueId));
    t_json.insert("isReadOnly", m_impl->isReadOnly);
}


} // namespace photon
