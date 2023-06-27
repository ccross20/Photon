#include "wheelslotcapability.h"
#include "fixture/fixturechannel.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturewheel.h"

namespace photon {

class WheelSlotCapability::Impl
{
public:
    QString wheelName;
    int slotNumber;
    FixtureWheelSlot *wheelSlot = nullptr;
};

WheelSlotCapability::WheelSlotCapability() : FixtureCapability(DMXRange(),Capability_WheelSlot),m_impl(new Impl)
{

}

WheelSlotCapability::~WheelSlotCapability()
{
    delete m_impl;
}

QString WheelSlotCapability::wheelName() const
{
    return m_impl->wheelName;
}

int WheelSlotCapability::slotNumber() const
{
    return m_impl->slotNumber;
}

FixtureWheelSlot *WheelSlotCapability::wheelSlot() const
{
    return m_impl->wheelSlot;
}

void WheelSlotCapability::selectSlot(DMXMatrix &t_matrix) const
{
    t_matrix.setValue(channel()->universe() - 1, channel()->universalChannelNumber(),range().middle(),1.0);
}


void WheelSlotCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

    m_impl->wheelName = channel()->name();
    m_impl->slotNumber = t_json.value("slotNumber").toInt();

    for(auto wheel : fixture()->wheels())
    {
        if(wheel->name().toLower() == m_impl->wheelName.toLower())
        {
            m_impl->wheelSlot = wheel->slot(m_impl->slotNumber-1);
            break;
        }
    }
}

} // namespace photon
