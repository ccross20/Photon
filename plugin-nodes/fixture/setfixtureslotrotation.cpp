#include "setfixtureslotrotation.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/capability/wheelslotrotationcapability.h"
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


    m_nameParam = new keira::StringParameter("nameInput","Name", "color");
    addParameter(m_nameParam);

}

void SetFixtureSlotRotation::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        float rotationAmount = m_angleParam->value().toFloat();

        bool useAngle = m_modeParam->value().toInt() == 0;
        QString name = m_nameParam->value().toString().toLower();

        if(!name.isEmpty())
        {
            auto allCapabilities = context->fixture->findCapability(CapabilityType::Capability_WheelSlotRotation, name);

            //qDebug() << name << allCapabilities.length();

            for(auto capability : allCapabilities)
            {
                auto wheelSlot = static_cast<WheelSlotRotationCapability*>(capability);

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

}

} // namespace photon
