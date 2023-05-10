#include "dimmercapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

class DimmerCapability::Impl
{
public:

};

DimmerCapability::DimmerCapability(DMXRange range) : FixtureCapability(range, Capability_Dimmer),m_impl(new Impl)
{

}

DimmerCapability::~DimmerCapability()
{
    delete m_impl;
}

void DimmerCapability::setPercent(double value, DMXMatrix &t_matrix, double t_blend)
{
    t_matrix.setValuePercent(fixture()->universe()-1,channel()->universalChannelNumber(), value, t_blend);
}

void DimmerCapability::setLumens(double value, DMXMatrix &t_matrix, double blend)
{

}

double DimmerCapability::getPercent(const DMXMatrix &t_matrix)
{
    return t_matrix.valuePercent(fixture()->universe()-1,channel()->universalChannelNumber());
}




void DimmerCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

}


} // namespace photon
