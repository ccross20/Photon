#include "setfixtureslot.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/fixturewheel.h"
#include "fixture/capability/wheelslotcapability.h"

namespace photon {

keira::NodeInformation SetFixtureSlot::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureSlot;});
    toReturn.name = "Set Fixture Slot";
    toReturn.nodeId = "photon.plugin.node.set-fixture-slot";
    toReturn.categories = {"Fixture"};

    return toReturn;
}

SetFixtureSlot::SetFixtureSlot(): keira::Node("photon.plugin.node.set-fixture-slot")
{

}

void SetFixtureSlot::createParameters()
{

    m_modeParam = new keira::OptionParameter("mode","Mode",{"Name","Number"},0);
    addParameter(m_modeParam);

    m_wheelParam = new keira::IntegerParameter("wheelInput","Wheel Number", 1);
    m_wheelParam->setMinimum(1);
    m_wheelParam->setMaximum(20);
    addParameter(m_wheelParam);

    m_slotParam = new keira::IntegerParameter("slotInput","Slot Number", 1);
    m_slotParam->setMinimum(1);
    m_slotParam->setMaximum(100);
    addParameter(m_slotParam);

    m_wheelNameParam = new keira::StringParameter("nameInput","Wheel Name", "color");
    addParameter(m_wheelNameParam);

}

void SetFixtureSlot::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto allSlots = context->fixture->findCapability(Capability_WheelSlot);

        int wheelIndex = m_wheelParam->value().toInt() - 1;
        int slotNum = m_slotParam->value().toInt();

        //qDebug() << "Slot " << slotNum << allSlots.length();

        QString wheelName = m_wheelNameParam->value().toString();

        if(m_modeParam->value().toInt() == 1 && wheelIndex < context->fixture->wheels().length())
            wheelName = context->fixture->wheels()[wheelIndex]->name();
        wheelName = wheelName.toLower();


        for(auto curSlot : allSlots)
        {
            auto wheelSlot = static_cast<WheelSlotCapability*>(curSlot);


            if(wheelSlot->wheelName().toLower() == wheelName)
            {
                if(wheelSlot->slotNumber() == slotNum)
                {
                    wheelSlot->selectSlot(context->dmxMatrix);
                    return;
                }

            }
        }
    }

}

} // namespace photon
