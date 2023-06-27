#include "setfixturestrobe.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/shutterstrobecapability.h"

namespace photon {

keira::NodeInformation SetFixtureStrobe::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureStrobe;});
    toReturn.name = "Set Fixture Strobe";
    toReturn.nodeId = "photon.plugin.node.set-fixture-strobe";
    toReturn.categories = {"Fixture"};

    return toReturn;
}

SetFixtureStrobe::SetFixtureStrobe() : keira::Node("photon.plugin.node.set-fixture-strobe")
{

}

void SetFixtureStrobe::createParameters()
{
    m_speedParam = new keira::DecimalParameter("speedInput","Speed", .5);
    m_speedParam->setMinimum(0);
    m_speedParam->setMaximum(1.0);
    addParameter(m_speedParam);

    m_modeParam = new keira::IntegerParameter("modeInput","Mode", 2);
    m_modeParam->setMinimum(0);
    m_modeParam->setMaximum(2);
    addParameter(m_modeParam);

}

void SetFixtureStrobe::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto strobes = context->fixture->findCapability(Capability_Strobe);
        double speed = m_speedParam->value().toDouble();
        ShutterStrobeCapability::ShutterEffect mode = static_cast<ShutterStrobeCapability::ShutterEffect>(m_modeParam->value().toInt());

        for(auto curStrobe : strobes)
        {
            auto strobe = static_cast<ShutterStrobeCapability*>(curStrobe);

            if(strobe->shutterEffect() == mode)
            {
                strobe->setSpeedPercent(speed, context->dmxMatrix);
                return;
            }
        }
    }

}

} // namespace photon
