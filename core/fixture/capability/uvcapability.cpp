#include "uvcapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class UVCapability::Impl
{
public:

};

UVCapability::UVCapability(DMXRange range) : FixtureCapability(range, Capability_UV), m_impl(new Impl)
{

}

UVCapability::~UVCapability()
{
    delete m_impl;
}

void UVCapability::setPercent(double value, DMXMatrix &t_matrix, double t_blend, DMXTimeMachine *t_timeMachine)
{
    t_matrix.setValuePercent(channel(), value, t_blend, t_timeMachine);
}

double UVCapability::getPercent(const DMXMatrix &t_matrix) const
{
    return t_matrix.valuePercent(fixture()->universe()-1, channel()->universalChannelNumber());
}

void UVCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);
}

} // namespace photon
