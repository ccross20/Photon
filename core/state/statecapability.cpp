#include <QColor>
#include "statecapability.h"
#include "fixture/fixture.h"

namespace photon {

class StateCapability::Impl
{
public:
    QVector<ChannelInfo> availableChannels;
    QVector<QVariant> values;
    QString name;
    CapabilityType type;
    QByteArray uniqueId;
    bool isEnabled = true;
    int index = 0;
};

StateCapability::StateCapability(CapabilityType t_type):m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->type = t_type;
}

StateCapability::~StateCapability()
{
    delete m_impl;
}

QString StateCapability::name() const
{
    return m_impl->name;
}

void StateCapability::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QVector<FixtureCapability*> StateCapability::getFixtureCapabilities(const StateEvaluationContext &t_context) const
{
    auto fixture = t_context.fixture;
    if(!fixture)
        return QVector<FixtureCapability*>();

    return fixture->findCapability(fixtureCapabilityType(), index());
}

float StateCapability::getChannelFloat(const StateEvaluationContext &t_context, uint t_index) const
{
    bool success = false;
    auto val = t_context.channelValues[channelId(t_index)].toFloat(&success);
    if(success)
        return val;
    return 0.0f;
}

QColor StateCapability::getChannelColor(const StateEvaluationContext &t_context, uint t_index) const
{
    return t_context.channelValues[channelId(t_index)].value<QColor>();
}

int StateCapability::getChannelInteger(const StateEvaluationContext &t_context, uint t_index) const
{
    bool success = false;
    auto val = t_context.channelValues[channelId(t_index)].toInt(&success);
    if(success)
        return val;
    return 0.0f;
}

QString StateCapability::getChannelString(const StateEvaluationContext &t_context, uint t_index) const
{
   return t_context.channelValues[channelId(t_index)].toString();
}

bool StateCapability::getChannelBool(const StateEvaluationContext &t_context, uint t_index) const
{
    return t_context.channelValues[channelId(t_index)].toBool();
}

void StateCapability::resetChannelToDefault(uint t_index)
{
    if(t_index >= m_impl->values.size())
        return;
    m_impl->values[t_index] = m_impl->availableChannels[t_index].defaultValue;
}

void StateCapability::setChannelValue(uint t_index, const QVariant &t_value)
{
    if(t_index >= m_impl->values.size())
        return;
    m_impl->values[t_index] = t_value;
}

QVariant StateCapability::getChannelValue(uint t_index) const
{
    if(t_index >= m_impl->values.size())
        return 0;
    return m_impl->values[t_index];
}

CapabilityType StateCapability::capabilityType() const
{
    return m_impl->type;
}

CapabilityType StateCapability::fixtureCapabilityType() const
{
    return capabilityType();
}

QVector<ChannelInfo> StateCapability::availableChannels() const
{
    return m_impl->availableChannels;
}

QByteArray StateCapability::channelId(uint t_index) const
{
    return m_impl->uniqueId + "-" + t_index;
}

void StateCapability::initializeValues(StateEvaluationContext &t_context) const
{
    for(int i = 0; i < m_impl->values.length(); ++i)
    {
        t_context.channelValues.insert(channelId(i), m_impl->values[i]);
    }
}

void StateCapability::evaluate(const StateEvaluationContext &) const
{

}

void StateCapability::addAvailableChannel(const ChannelInfo &t_info)
{
    auto info = t_info;
    info.uniqueId = channelId(m_impl->availableChannels.length());
    m_impl->availableChannels.append(info);
    m_impl->values.append(info.defaultValue);
}

bool StateCapability::isEnabled() const
{
    return m_impl->isEnabled;
}

void StateCapability::setIsEnabled(bool t_value)
{
    m_impl->isEnabled = t_value;
}


uint StateCapability::index() const
{
    return m_impl->index;
}

void StateCapability::setIndex(uint t_value)
{
    m_impl->index = t_value;
}

void StateCapability::restore(Project &t_project)
{

}

void StateCapability::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    m_impl->isEnabled = t_json.value("isEnabled").toBool();
    m_impl->index = t_json.value("index").toInt();
    m_impl->type = static_cast<CapabilityType>(t_json.value("type").toInt());

    int index = 0;
    for(auto &channel : m_impl->availableChannels)
        channel.uniqueId = channelId(index++);

    if(t_json.contains("values"))
    {
        QJsonArray valueArray = t_json.value("values").toArray();
        int index = 0;
        for(auto value : valueArray)
        {
            setChannelValue(index++,value.toVariant());
        }
    }

}

void StateCapability::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("uniqueId", QString{m_impl->uniqueId});
    t_json.insert("isEnabled", m_impl->isEnabled);
    t_json.insert("index", m_impl->index);
    t_json.insert("type", m_impl->type);

    QJsonArray valueArray;
    for(const auto &value : m_impl->values)
    {
        valueArray.append(QJsonValue::fromVariant(value));
    }
    t_json.insert("values", valueArray);

}

} // namespace photon
