#include <QColor>
#include "colorstate.h"
#include "fixture/capability/colorcapability.h"

namespace photon {

ColorState::ColorState() : StateCapability(CapabilityType::Capability_Color)
{
    setName("Color");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeColor, "Color","Color of the light",0));
}

void ColorState::evaluate(const StateEvaluationContext &t_context) const
{
    auto colors = getFixtureCapabilities(t_context);
    QColor color = getChannelColor(t_context,0);

    for(auto curColor : colors)
    {
        auto c = static_cast<ColorCapability*>(curColor);
        c->setColor(color,t_context.dmxMatrix, t_context.strength);
        return;
    }
}

} // namespace photon
