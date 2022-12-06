#include "setfixturetilt.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/tiltcapability.h"

namespace photon {

keira::NodeInformation SetFixtureTilt::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureTilt;});
    toReturn.name = "Set Fixture Tilt";
    toReturn.nodeId = "photon.plugin.node.set-fixture-tilt";

    return toReturn;
}

SetFixtureTilt::SetFixtureTilt() : keira::Node("photon.plugin.node.set-fixture-tilt")
{

}

void SetFixtureTilt::createParameters()
{
    m_angleParam = new keira::DecimalParameter("angleInput","Angle", 0.0);
    m_angleParam->setMinimum(-1.0);
    m_angleParam->setMaximum(1.0);
    addParameter(m_angleParam);

    m_capabilityParam = new keira::IntegerParameter("capabilityIndex","Capability", 0);
    m_capabilityParam->setMinimum(0);
    m_capabilityParam->setMaximum(100);
    addParameter(m_capabilityParam);

    m_blendParam = new keira::DecimalParameter("blendInput","Blend", 1.0);
    m_blendParam->setMinimum(0.0);
    m_blendParam->setMaximum(1.0);
    addParameter(m_blendParam);
}

void SetFixtureTilt::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto pans = context->fixture->findCapability(Capability_Tilt);

        int index = m_capabilityParam->value().toInt();
        if(index < pans.length())
        {
            static_cast<TiltCapability*>(pans[index])->setAnglePercent(m_angleParam->value().toDouble() * m_blendParam->value().toDouble(), context->dmxMatrix);
        }
    }

}

} // namespace photon
