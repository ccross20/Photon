#include <QColor>
#include <QImage>
#include "fixturewheel.h"

namespace photon {

class FixtureWheelSlot::Impl
{
public:
    SlotType type;
    int index;
};

FixtureWheelSlot::FixtureWheelSlot(FixtureWheelSlot::SlotType t_type, int t_index):m_impl(new Impl)
{
    m_impl->type = t_type;
    m_impl->index = t_index;
}

FixtureWheelSlot::~FixtureWheelSlot()
{
    delete m_impl;
}


FixtureWheelSlot::SlotType FixtureWheelSlot::type() const
{
    return m_impl->type;
}

int FixtureWheelSlot::index() const
{
    return m_impl->index;
}

void FixtureWheelSlot::readFromJson(const QJsonObject & t_json)
{
    //m_impl->type = static_cast<SlotType>(t_json.value("type").toInt());
}


class ColorSlot::Impl
{
public:
    QVector<QColor> colors;
};


ColorSlot::ColorSlot(int t_index):FixtureWheelSlot(Slot_Color, t_index),m_impl(new Impl)
{

}

ColorSlot::~ColorSlot()
{
    delete m_impl;
}

QColor ColorSlot::color() const
{
    if(m_impl->colors.isEmpty())
        return Qt::white;
    return m_impl->colors.front();
}

QVector<QColor> ColorSlot::colors() const
{
    return m_impl->colors;
}

void ColorSlot::readFromJson(const QJsonObject &t_json)
{
    FixtureWheelSlot::readFromJson(t_json);

    auto colorArray = t_json.value("colors").toArray();

    for(auto color : colorArray)
    {
        m_impl->colors.append(color.toString());
    }
}

class GoboSlot::Impl
{
public:
    QImage image;
    QString path;
};

GoboSlot::GoboSlot(int t_index) : FixtureWheelSlot(SlotType::Slot_Gobo, t_index),m_impl(new Impl)
{

}

GoboSlot::~GoboSlot()
{
    delete m_impl;
}

QImage GoboSlot::image() const
{
    return m_impl->image;
}

void GoboSlot::setImagePath(const QString &t_path)
{
    m_impl->path = t_path;
}

QString GoboSlot::imagePath() const
{
    return m_impl->path;
}

void GoboSlot::readFromJson(const QJsonObject &t_json)
{

}








class FixtureWheel::Impl
{
public:
    QVector<FixtureWheelSlot*> wheelSlots;
    QString name;
};

FixtureWheel::FixtureWheel(const QString &t_name):m_impl(new Impl)
{
    m_impl->name = t_name;
}

FixtureWheel::~FixtureWheel()
{
    delete m_impl;
}

QString FixtureWheel::name()
{
    return m_impl->name;
}

FixtureWheelSlot *FixtureWheel::slot(int t_index) const
{
    if(t_index >= m_impl->wheelSlots.length())
        return nullptr;
    return m_impl->wheelSlots[t_index];
}

int FixtureWheel::slotCount() const
{
    return m_impl->wheelSlots.count();
}

const QVector<FixtureWheelSlot*> &FixtureWheel::allSlots() const
{
    return m_impl->wheelSlots;
}


void FixtureWheel::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    auto slotArray = t_json.value("slots").toArray();

    for(auto slotVal : slotArray)
    {
        auto slotObj = slotVal.toObject();
        auto slotType = slotObj.value("type").toString().toLower();
        FixtureWheelSlot *newSlot = nullptr;

        if(slotType == "open")
            newSlot = new FixtureWheelSlot(FixtureWheelSlot::Slot_Open, m_impl->wheelSlots.length());
        else if(slotType == "color")
            newSlot = new ColorSlot(m_impl->wheelSlots.length());
        else if(slotType == "gobo")
            newSlot = new GoboSlot(m_impl->wheelSlots.length());
        else if(slotType == "animationgobostart")
            newSlot = new FixtureWheelSlot(FixtureWheelSlot::Slot_AnimationGoboStart, m_impl->wheelSlots.length());
        else if(slotType == "animationgoboend")
            newSlot = new FixtureWheelSlot(FixtureWheelSlot::Slot_AnimationGoboEnd, m_impl->wheelSlots.length());


        if(newSlot)
        {
            newSlot->readFromJson(slotObj);
            m_impl->wheelSlots.append(newSlot);
        }
    }
}

} // namespace photon
