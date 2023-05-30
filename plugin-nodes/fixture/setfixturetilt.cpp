#include "setfixturetilt.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"

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
    //m_angleParam->setMinimum(-1.0);
    //m_angleParam->setMaximum(1.0);
    addParameter(m_angleParam);

    m_capabilityParam = new keira::IntegerParameter("capabilityIndex","Capability", 0);
    m_capabilityParam->setMinimum(0);
    m_capabilityParam->setMaximum(100);
    addParameter(m_capabilityParam);

    m_modeParam = new keira::IntegerParameter("mode","Mode", 0);
    m_modeParam->setMinimum(0);
    m_modeParam->setMaximum(1);
    addParameter(m_modeParam);

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
            if(m_modeParam->value().toInt() == 0)
                static_cast<AngleCapability*>(pans[index])->setAnglePercent(m_angleParam->value().toDouble(), context->dmxMatrix, m_blendParam->value().toDouble());
            else
                static_cast<AngleCapability*>(pans[index])->setAngleDegreesCentered(m_angleParam->value().toDouble(), context->dmxMatrix, m_blendParam->value().toDouble());
        }
    }

}

} // namespace photon
