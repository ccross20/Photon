#include <QHash>
#include "setfixtureslotrotation.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/capability/wheelslotrotationcapability.h"
#include "fixture/fixturechannel.h"
#include "fixture/fixture.h"

namespace photon {

keira::NodeInformation SetFixtureSlotRotation::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureSlotRotation;});
    toReturn.name = "Set Fixture Slot Rotation";
    toReturn.nodeId = "photon.plugin.node.set-fixture-slot-rotation";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

SetFixtureSlotRotation::SetFixtureSlotRotation() : keira::Node("photon.plugin.node.set-fixture-slot-rotation"){}

void SetFixtureSlotRotation::createParameters()
{

    m_modeParam = new keira::OptionParameter("mode","Mode",{"Indexed","Continuous"},0);
    addParameter(m_modeParam);

    m_angleParam = new keira::DecimalParameter("amount","Amount", 0);
    addParameter(m_angleParam);

    // Which rotating (gobo) wheel, by order among the fixture's slot-rotation channels.
    m_wheelParam = new keira::OptionParameter("wheel","Wheel",
        {"Gobo Wheel 1","Gobo Wheel 2","Gobo Wheel 3"}, 0);
    addParameter(m_wheelParam);

}

void SetFixtureSlotRotation::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        float rotationAmount = m_angleParam->value().toFloat();

        bool useAngle = m_modeParam->value().toInt() == 0;

        // Group slot-rotation capabilities by channel (one channel == one wheel),
        // preserving order, then pick the wheel chosen in the dropdown.
        auto allCapabilities = context->fixture->findCapability(CapabilityType::Capability_WheelSlotRotation);

        QVector<FixtureChannel*> channelOrder;
        QHash<FixtureChannel*, QVector<WheelSlotRotationCapability*>> byChannel;
        for(auto capability : allCapabilities)
        {
            auto cap = static_cast<WheelSlotRotationCapability*>(capability);
            FixtureChannel *ch = cap->channel();
            if(!byChannel.contains(ch))
                channelOrder.append(ch);
            byChannel[ch].append(cap);
        }

        const int wheelIdx = m_wheelParam->value().toInt();
        if(wheelIdx < 0 || wheelIdx >= channelOrder.size())
            return;

        for(auto wheelSlot : byChannel[channelOrder[wheelIdx]])
        {
            if(wheelSlot->supportsAngle() && useAngle)
            {
                wheelSlot->setAngleDegrees(rotationAmount,context->dmxMatrix,1.0);
                return;
            }
            else
            {
                if(rotationAmount > 0 && wheelSlot->maxSpeed() > 0)
                {
                    wheelSlot->setSpeed(rotationAmount,context->dmxMatrix,1.0);
                    return;
                }
                else if(rotationAmount < 0 && wheelSlot->maxSpeed() < 0)
                {
                    wheelSlot->setSpeed(rotationAmount,context->dmxMatrix,1.0);
                    return;
                }
                else if(rotationAmount == 0 && wheelSlot->isStop())
                {
                    wheelSlot->selectCapability(context->dmxMatrix);
                    return;
                }
            }
        }
    }

}

} // namespace photon
