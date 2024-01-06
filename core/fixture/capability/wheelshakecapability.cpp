#include "wheelshakecapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixturechannel.h"

namespace photon {

class WheelShakeCapability::Impl
{
public:
    double shakeStart = 0;
    double shakeStop = 0;
};

WheelShakeCapability::WheelShakeCapability() : WheelSlotCapability(DMXRange(),Capability_WheelShake),m_impl(new Impl)
{

}

WheelShakeCapability::~WheelShakeCapability()
{
    delete m_impl;
}

double WheelShakeCapability::shakeSpeedStart() const
{
    return m_impl->shakeStart;
}

double WheelShakeCapability::shakeSpeedEnd() const
{
    return m_impl->shakeStop;
}

void WheelShakeCapability::setSpeed(double value, DMXMatrix &t_matrix, double t_blend)
{
    if(value < 0)
        value *= -1.0;

    if(m_impl->shakeStop > m_impl->shakeStart)
        value = 1.0 - value;

    t_matrix.setRangeMappedValuePercent(channel(), value, range());
}

void WheelShakeCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    WheelSlotCapability::readFromOpenFixtureJson(t_json);


    FixtureCapability::constantPercent(t_json.value("shakeSpeedStart").toString(), &m_impl->shakeStart);
    FixtureCapability::constantPercent(t_json.value("shakeSpeedStop").toString(), &m_impl->shakeStop);

}

} // namespace photon
