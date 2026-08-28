#include <QHash>
#include "prismstate.h"
#include "fixture/capability/prismcapability.h"
#include "fixture/capability/prismrotationcapability.h"
#include "fixture/fixturechannel.h"
#include "fixture/fixture.h"

namespace photon {

PrismState::PrismState() : StateCapability(CapabilityType::Capability_Prism)
{
    prismOptions = QStringList{"Prism 1", "Prism 2", "Prism 3"};
    modeOptions = QStringList{"Continuous", "Indexed"};

    setName("Prism");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Use Prism", "Engage the prism", false));

    auto prismInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Prism", "Which prism to use", 0);
    prismInfo.options = prismOptions;
    addAvailableChannel(prismInfo);

    auto modeInfo = ChannelInfo(ChannelInfo::ChannelTypeIntegerStep, "Mode", "Rotate continuously or hold an angle", 0);
    modeInfo.options = modeOptions;
    addAvailableChannel(modeInfo);

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Amount", "Rotation speed, or angle in Indexed mode", 0));
}

void PrismState::evaluate(const StateEvaluationContext &t_context) const
{
    if(!t_context.fixture)
        return;

    if(!getChannelBool(t_context, 0))
        return;   // prism off - leave its channels at rest (default DMX = off)

    const int prismIndex = getChannelInteger(t_context, 1);
    const bool useAngle = getChannelInteger(t_context, 2) == 1;
    const float rotationAmount = getChannelFloat(t_context, 3);

    // Engage the prism's on/off channel. Fixtures like the Dartz gate the prism
    // behind a separate "Prism N" channel - driving the rotation channel alone
    // isn't enough. One Prism capability per prism channel, so index directly.
    auto prismCaps = t_context.fixture->findCapability(CapabilityType::Capability_Prism);
    if(prismIndex >= 0 && prismIndex < prismCaps.size())
        static_cast<PrismCapability*>(prismCaps[prismIndex])->selectCapability(t_context.dmxMatrix);

    // Group rotation capabilities by channel (one channel == one prism) and keep
    // the channel order, so "Prism 2" means the second physical prism channel.
    // Selecting by channel ordinal rather than by channel name - as the other
    // rotation states do - is what keeps this working across fixture profiles
    // that name their prism channels differently.
    auto allRotation = t_context.fixture->findCapability(CapabilityType::Capability_PrismRotation);
    QVector<FixtureChannel*> channelOrder;
    QHash<FixtureChannel*, QVector<PrismRotationCapability*>> byChannel;
    for(auto capability : allRotation)
    {
        auto cap = static_cast<PrismRotationCapability*>(capability);
        FixtureChannel *channel = cap->channel();
        if(!byChannel.contains(channel))
            channelOrder.append(channel);
        byChannel[channel].append(cap);
    }

    if(prismIndex < 0 || prismIndex >= channelOrder.size())
        return;   // selected prism has no rotation channel (still engaged above)

    for(auto prismCapability : byChannel[channelOrder[prismIndex]])
    {
        if(prismCapability->supportsAngle() && useAngle)
        {
            prismCapability->setAngleDegrees(rotationAmount, t_context.dmxMatrix, t_context.strength);
            return;
        }

        if(rotationAmount > 0 && prismCapability->maxSpeed() > 0)
        {
            prismCapability->setSpeed(rotationAmount, t_context.dmxMatrix, t_context.strength);
            return;
        }
        if(rotationAmount < 0 && prismCapability->maxSpeed() < 0)
        {
            prismCapability->setSpeed(rotationAmount, t_context.dmxMatrix, t_context.strength);
            return;
        }
        if(rotationAmount == 0 && prismCapability->isStop())
        {
            prismCapability->selectCapability(t_context.dmxMatrix);
            return;
        }
    }
}

} // namespace photon
