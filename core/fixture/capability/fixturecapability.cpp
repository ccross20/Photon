#include <QRegularExpression>
#include "fixturecapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixturechannel.h"
#include "fixture/fixture.h"

namespace photon {

class FixtureCapability::Impl
{
public:
    DMXRange range;
    FixtureChannel *channel = nullptr;
    FixtureChannel *fineChannel = nullptr;
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

bool FixtureCapability::isValid(const DMXMatrix &t_matrix) const
{
    return m_impl->range.contains(t_matrix.value(fixture()->universe()-1,channel()->universalChannelNumber()));
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

    if(constantPercent(value,valueWithoutUnits))
        return Unit_Percent;

    return Unit_Unknown;
}

FixtureUnit FixtureCapability::rotationSpeed(const QString &value, double *valueWithoutUnits)
{

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

    if(constantPercent(value,valueWithoutUnits))
        return Unit_Percent;

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

    if(constantPercent(value,valueWithoutUnits))
        return Unit_Percent;

    return Unit_Unknown;
}

FixtureUnit FixtureCapability::distance(const QString &value, double *valueWithoutUnits)
{
    int degIndex = value.indexOf("m");
    if(degIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(degIndex).toDouble();
        return Unit_Meters;
    }

    int percentIndex = value.indexOf("%");
    if(percentIndex >= 0)
    {
        *valueWithoutUnits = QStringView{value}.left(percentIndex).toDouble();
        return Unit_Percent;
    }

    if(constantPercent(value,valueWithoutUnits))
        return Unit_Percent;

    return Unit_Unknown;
}

bool FixtureCapability::constantPercent(const QString &constant, double *value)
{
    QString lowered = constant.toLower();
    if(lowered == "near"){*value = .01; return true;}
    else if(lowered == "far"){*value = 1; return true;}
    else if(lowered == "off"){*value = 0; return true;}
    else if(lowered == "dark"){*value = .01; return true;}
    else if(lowered == "bright"){*value = 1; return true;}
    else if(lowered == "closed"){*value = 0; return true;}
    else if(lowered == "narrow"){*value = .01; return true;}
    else if(lowered == "wide"){*value = 1; return true;}

    else if(lowered == "fast reverse"){*value = -1; return true;}
    else if(lowered == "slow reverse"){*value = -.01; return true;}
    else if(lowered == "stop"){*value = 0; return true;}
    else if(lowered == "slow"){*value = .01; return true;}
    else if(lowered == "fast"){*value = 1; return true;}

    else if(lowered == "fast ccw"){*value = -1; return true;}
    else if(lowered == "slow ccw"){*value = -.01; return true;}
    else if(lowered == "stop"){*value = 0; return true;}
    else if(lowered == "slow cw"){*value = .01; return true;}
    else if(lowered == "fast cw"){*value = 1; return true;}


    return false;
}


} // namespace photon
