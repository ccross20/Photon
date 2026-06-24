#include <QHash>
#include "setfixtureprismrotation.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/capability/prismrotationcapability.h"
#include "fixture/capability/prismcapability.h"
#include "fixture/fixturechannel.h"
#include "fixture/fixture.h"

namespace photon {

keira::NodeInformation SetFixturePrismRotation::info()
{
    keira::NodeInformation toReturn([](){return new SetFixturePrismRotation;});
    toReturn.name = "Set Fixture Prism Rotation";
    toReturn.nodeId = "photon.plugin.node.set-fixture-prism-rotation";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

SetFixturePrismRotation::SetFixturePrismRotation() : keira::Node("photon.plugin.node.set-fixture-prism-rotation"){}

void SetFixturePrismRotation::createParameters()
{

    // Which prism, by order among the fixture's prism channels.
    m_prismParam = new keira::OptionParameter("prism","Prism",{"Prism 1","Prism 2","Prism 3"},0);
    addParameter(m_prismParam);

    m_modeParam = new keira::OptionParameter("mode","Mode",{"Continuous","Indexed"},0);
    addParameter(m_modeParam);

    m_angleParam = new keira::DecimalParameter("amount","Amount", 0);
    addParameter(m_angleParam);

    m_usePrismParam = new keira::BooleanParameter("usePrism","Use Prism", true);
    addParameter(m_usePrismParam);

}

void SetFixturePrismRotation::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(!context->fixture)
        return;

    if(!m_usePrismParam->value().toBool())
        return;   // prism off → leave its channels at rest (default DMX = off)

    const float rotationAmount = m_angleParam->value().toFloat();
    const bool useAngle = m_modeParam->value().toInt() == 1;
    const int prismIdx = m_prismParam->value().toInt();

    // Engage the prism on/off channel for the selected prism (fixtures like the dartz
    // gate the prism on a separate "Prism N" channel; the rotation channel alone is
    // not enough). One Prism capability per prism channel, so index directly.
    auto prismCaps = context->fixture->findCapability(CapabilityType::Capability_Prism);
    if(prismIdx < prismCaps.size())
        static_cast<PrismCapability*>(prismCaps[prismIdx])->selectCapability(context->dmxMatrix);

    // Group rotation capabilities by channel (one channel == one prism), in order,
    // then drive the channel for the selected prism.
    auto allRotation = context->fixture->findCapability(CapabilityType::Capability_PrismRotation);
    QVector<FixtureChannel*> channelOrder;
    QHash<FixtureChannel*, QVector<PrismRotationCapability*>> byChannel;
    for(auto capability : allRotation)
    {
        auto cap = static_cast<PrismRotationCapability*>(capability);
        FixtureChannel *ch = cap->channel();
        if(!byChannel.contains(ch))
            channelOrder.append(ch);
        byChannel[ch].append(cap);
    }

    if(prismIdx < 0 || prismIdx >= channelOrder.size())
        return;   // selected prism has no rotation channel (still engaged above)

    for(auto prismCapability : byChannel[channelOrder[prismIdx]])
    {
        if(prismCapability->supportsAngle() && useAngle)
        {
            prismCapability->setAngleDegrees(rotationAmount,context->dmxMatrix,1.0);
            return;
        }
        else
        {
            if(rotationAmount > 0 && prismCapability->maxSpeed() > 0)
            {
                prismCapability->setSpeed(rotationAmount,context->dmxMatrix,1.0);
                return;
            }
            else if(rotationAmount < 0 && prismCapability->maxSpeed() < 0)
            {
                prismCapability->setSpeed(rotationAmount,context->dmxMatrix,1.0);
                return;
            }
            else if(rotationAmount == 0 && prismCapability->isStop())
            {
                prismCapability->selectCapability(context->dmxMatrix);
                return;
            }
        }
    }

}

} // namespace photon
