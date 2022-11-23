#include "setfixturepan.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/pancapability.h"

namespace photon {

keira::NodeInformation SetFixturePan::info()
{
    keira::NodeInformation toReturn([](){return new SetFixturePan;});
    toReturn.name = "Set Fixture Pan";
    toReturn.nodeId = "photon.plugin.node.set-fixture-pan";

    return toReturn;
}

SetFixturePan::SetFixturePan() : keira::Node("photon.plugin.node.set-fixture-pan")
{

}

void SetFixturePan::createParameters()
{
    m_angleParam = new keira::DecimalParameter("angleInput","Angle", 0.0);
    m_angleParam->setMinimum(0.0);
    m_angleParam->setMaximum(1.0);
    addParameter(m_angleParam);

    m_capabilityParam = new keira::IntegerParameter("capabilityIndex","Capability", 0);
    m_capabilityParam->setMinimum(0);
    m_capabilityParam->setMaximum(100);
    addParameter(m_capabilityParam);
}

void SetFixturePan::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto pans = context->fixture->findCapability(Capability_Pan);

        int index = m_capabilityParam->value().toInt();
        if(index < pans.length())
        {
            static_cast<PanCapability*>(pans[index])->setAnglePercent(m_angleParam->value().toDouble(), context->dmxMatrix);
        }
    }

}

} // namespace photon
