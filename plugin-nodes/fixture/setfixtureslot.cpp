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
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

SetFixtureSlot::SetFixtureSlot(): keira::Node("photon.plugin.node.set-fixture-slot")
{

}

// Generic wheel roles offered in the dropdown; the label is resolved per-fixture
// (by slot type + ordinal) to the actual wheel name. "Custom" uses the text field.
static const QStringList kWheelRoles = {
    "Color Wheel", "Gobo Wheel 1", "Gobo Wheel 2", "Gobo Wheel 3",
    "Animation Wheel", "Custom"
};

void SetFixtureSlot::createParameters()
{
    m_wheelRoleParam = new keira::OptionParameter("wheelRole","Wheel", kWheelRoles, 1);
    addParameter(m_wheelRoleParam);

    m_wheelNameParam = new keira::StringParameter("nameInput","Custom Wheel", "color");
    addParameter(m_wheelNameParam);

    m_rotateModeParam = new keira::OptionParameter("rotateMode","Rotate Mode",{"Unknown","Indexed","Continuous"},0);
    addParameter(m_rotateModeParam);

    m_slotParam = new keira::IntegerParameter("slotInput","Slot Number", 1);
    m_slotParam->setMinimum(1);
    m_slotParam->setMaximum(100);
    addParameter(m_slotParam);
}

void SetFixtureSlot::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto allSlots = context->fixture->findCapability(Capability_WheelSlot);

        int slotNum = m_slotParam->value().toInt();

        auto rotateMode = static_cast<WheelSlotCapability::RotateMode>(m_rotateModeParam->value().toInt());

        // Resolve the chosen generic role to the fixture's actual wheel name; "Custom"
        // falls back to the free-text field.
        const int roleIdx = m_wheelRoleParam->value().toInt();
        const QString role = (roleIdx >= 0 && roleIdx < kWheelRoles.size()) ? kWheelRoles[roleIdx] : QString();
        QString wheelName = (role == "Custom" || role.isEmpty())
            ? m_wheelNameParam->value().toString()
            : context->fixture->resolveWheelName(role);
        wheelName = wheelName.toLower();

        for(auto curSlot : allSlots)
        {
            auto wheelSlot = static_cast<WheelSlotCapability*>(curSlot);


            if(wheelSlot->wheelName().toLower() == wheelName)
            {
                if(wheelSlot->slotNumber() == slotNum && wheelSlot->rotateMode() == rotateMode)
                {
                    wheelSlot->selectSlot(context->dmxMatrix);
                    return;
                }

            }
        }
    }

}

} // namespace photon
