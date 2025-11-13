#include <QColor>
#include "colorstate.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/fixture.h"

namespace photon {

ColorState::ColorState() : StateCapability(CapabilityType::Capability_Color)
{
    setName("Color");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeColor, "Color","Color of the light",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Name","Name of the capability",""));
}

void ColorState::evaluate(const StateEvaluationContext &t_context) const
{
    auto colors = getFixtureCapabilities(t_context);
    QColor color = getChannelColor(t_context,0);

    QString name = getChannelString(t_context,1);

    if(!name.isEmpty())
    {
        auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_Color, name);

        for(auto capability : allCapabilities)
        {
            auto c = static_cast<ColorCapability*>(capability);

            c->setColor(color,t_context.dmxMatrix, t_context.strength);
        }
    }
    else
    {
        for(auto curColor : colors)
        {
            auto c = static_cast<ColorCapability*>(curColor);
            c->setColor(color,t_context.dmxMatrix, t_context.strength);
        }
    }


}

} // namespace photon
