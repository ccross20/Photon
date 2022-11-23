#include <QRegularExpression>
#include "fixturecapability.h"
#include "fixture/fixturechannel.h"

namespace photon {

class FixtureCapability::Impl
{
public:
    DMXRange range;
    FixtureChannel *channel = nullptr;
    CapabilityType type;
};

FixtureCapability::FixtureCapability(DMXRange range, CapabilityType capability):m_impl(new Impl)
{
    m_impl->range = range;
    m_impl->type = capability;
}

FixtureCapability::~FixtureCapability()
{
    delete m_impl;
}

DMXRange FixtureCapability::range() const
{
    return m_impl->range;
}

Fixture *FixtureCapability::fixture() const
{
    return m_impl->channel->fixture();
}

FixtureChannel *FixtureCapability::channel() const
{
    return m_impl->channel;
}

void FixtureCapability::setChannel(FixtureChannel *t_channel)
{
    m_impl->channel = t_channel;
}

CapabilityType FixtureCapability::type() const
{
    return m_impl->type;
}

void FixtureCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    if(t_json.contains("dmxRange"))
    {
        auto rangeArray = t_json.value("dmxRange").toArray();

        if(rangeArray.size() == 2)
        {
            m_impl->range.start = rangeArray[0].toInt();
            m_impl->range.end = rangeArray[1].toInt();
        }
        else
        {
            qWarning() << "dmxRange is invalid";
        }
    }
}

void FixtureCapability::readFromJson(const QJsonObject &t_json)
{
    m_impl->range.start = t_json.value("rangeMin").toInt();
    m_impl->range.end = t_json.value("rangeMax").toInt();
    m_impl->type = static_cast<CapabilityType>(t_json.value("type").toInt());
}

void FixtureCapability::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("rangeMin", m_impl->range.start);
    t_json.insert("rangeMax", m_impl->range.end);
    t_json.insert("type", m_impl->type);
}

FixtureUnit FixtureCapability::speed(const QString &value, double *valueWithoutUnits)
{
    if(value == "fast")
    {
        *valueWithoutUnits = Speed_Fast;
        return Unit_Enum;
    }
    else if(value == "slow")
    {
        *valueWithoutUnits = Speed_Slow;
        return Unit_Enum;
    }
    else if(value == "stop")
    {
        *valueWithoutUnits = Speed_Stop;
        return Unit_Enum;
    }
    else if(value == "slow reverse")
    {
        *valueWithoutUnits = Speed_SlowReverse;
        return Unit_Enum;
    }
    else if(value == "fast reverse")
    {
        *valueWithoutUnits = Speed_FastReverse;
        return Unit_Enum;
    }

    int hertzIndex = value.indexOf("Hz");
    if(hertzIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(hertzIndex).toDouble();
        return Unit_Hertz;
    }

    int bpmIndex = value.indexOf("bpm");
    if(bpmIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(bpmIndex).toDouble();
        return Unit_BeatsPerMinute;
    }

    int percentIndex = value.indexOf("%");
    if(percentIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(percentIndex).toDouble();
        return Unit_Percent;
    }

    return Unit_Unknown;
}

FixtureUnit FixtureCapability::rotationSpeed(const QString &value, double *valueWithoutUnits)
{
    if(value == "fast CW")
    {
        *valueWithoutUnits = RotationSpeed_FastCW;
        return Unit_Enum;
    }
    else if(value == "slow CW")
    {
        *valueWithoutUnits = RotationSpeed_SlowCW;
        return Unit_Enum;
    }
    else if(value == "stop")
    {
        *valueWithoutUnits = RotationSpeed_Stop;
        return Unit_Enum;
    }
    else if(value == "slow CCW")
    {
        *valueWithoutUnits = RotationSpeed_SlowCCW;
        return Unit_Enum;
    }
    else if(value == "fast CCW")
    {
        *valueWithoutUnits = RotationSpeed_FastCCW;
        return Unit_Enum;
    }

    int hertzIndex = value.indexOf("Hz");
    if(hertzIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(hertzIndex).toDouble();
        return Unit_Hertz;
    }

    int rpmIndex = value.indexOf("rpm");
    if(rpmIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(rpmIndex).toDouble();
        return Unit_RoundsPerMinute;
    }

    int percentIndex = value.indexOf("%");
    if(percentIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(percentIndex).toDouble();
        return Unit_Percent;
    }

    return Unit_Unknown;
}

FixtureUnit FixtureCapability::rotationAngle(const QString &value, double *valueWithoutUnits)
{
    int degIndex = value.indexOf("deg");
    if(degIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(degIndex).toDouble();
        return Unit_Degrees;
    }

    int percentIndex = value.indexOf("%");
    if(percentIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(percentIndex).toDouble();
        return Unit_Percent;
    }

    return Unit_Unknown;
}


} // namespace photon
