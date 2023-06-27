#include "setfixturefocus.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"

namespace photon {

keira::NodeInformation SetFixtureFocus::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureFocus;});
    toReturn.name = "Set Fixture Focus";
    toReturn.nodeId = "photon.plugin.node.set-fixture-focus";
    toReturn.categories = {"Fixture"};

    return toReturn;
}

SetFixtureFocus::SetFixtureFocus() : keira::Node("photon.plugin.node.set-fixture-focus")
{

}

void SetFixtureFocus::createParameters()
{
    m_angleParam = new keira::DecimalParameter("angleInput","Angle", 0.0);
    m_angleParam->setMinimum(0);
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

void SetFixtureFocus::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto pans = context->fixture->findCapability(Capability_Focus);

        int index = m_capabilityParam->value().toInt();
        if(index < pans.length())
        {
            static_cast<AngleCapability*>(pans[index])->setAnglePercent(m_angleParam->value().toDouble(), context->dmxMatrix, context->strength * m_blendParam->value().toDouble());
        }
    }

}

} // namespace photon
