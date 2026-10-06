#include "ctocapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class CTOCapability::Impl
{
public:

};

CTOCapability::CTOCapability(DMXRange range) : FixtureCapability(range, Capability_CTO), m_impl(new Impl)
{

}

CTOCapability::~CTOCapability()
{
    delete m_impl;
}

void CTOCapability::setPercent(double value, DMXMatrix &t_matrix, double t_blend)
{
    t_matrix.setValuePercent(channel(), value, t_blend);
}

double CTOCapability::getPercent(const DMXMatrix &t_matrix) const
{
    return t_matrix.valuePercent(fixture()->universe()-1, channel()->universalChannelNumber());
}

void CTOCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);
}

} // namespace photon
