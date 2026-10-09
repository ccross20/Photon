#include <algorithm>
#include "fogcapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

FogCapability::FogCapability(DMXRange range) : FixtureCapability(range, Capability_Fog)
{
}

void FogCapability::setOn(bool t_on, DMXMatrix &t_matrix) const
{
    FixtureChannel *dmxChannel = channel();
    if(!dmxChannel || !dmxChannel->isValid())
        return;
    // With no dmxRange in the definition the range is the whole channel.
    const int top = range().end;
    t_matrix.setValue(dmxChannel->universe() - 1, dmxChannel->universalChannelNumber(), uchar(t_on ? top : 0));
}

bool FogCapability::isOn(const DMXMatrix &t_matrix) const
{
    FixtureChannel *dmxChannel = channel();
    if(!dmxChannel || !dmxChannel->isValid())
        return false;
    const int value = t_matrix.valueInt(dmxChannel->universe() - 1, dmxChannel->universalChannelNumber());
    return value >= std::max(int(range().start), 1);
}

} // namespace photon
