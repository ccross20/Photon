#include "setfixturedimmer.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/dimmercapability.h"

namespace photon {

keira::NodeInformation SetFixtureDimmer::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureDimmer;});
    toReturn.name = "Set Fixture Dimmer";
    toReturn.nodeId = "photon.plugin.node.set-fixture-dimmer";

    return toReturn;
}

SetFixtureDimmer::SetFixtureDimmer() : keira::Node("photon.plugin.node.set-fixture-dimmer")
{

}

void SetFixtureDimmer::createParameters()
{
    m_intensityParam = new keira::DecimalParameter("dimmerInput","Intensity", 0.0);
    m_intensityParam->setMinimum(0.0);
    m_intensityParam->setMaximum(1.0);
    addParameter(m_intensityParam);

    m_capabilityParam = new keira::IntegerParameter("capabilityIndex","Capability", 0);
    m_capabilityParam->setMinimum(0);
    m_capabilityParam->setMaximum(100);
    addParameter(m_capabilityParam);

    m_blendParam = new keira::DecimalParameter("blendInput","Blend", 1.0);
    m_blendParam->setMinimum(0.0);
    m_blendParam->setMaximum(1.0);
    addParameter(m_blendParam);
}

void SetFixtureDimmer::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto dimmers = context->fixture->findCapability(Capability_Dimmer);

        int index = m_capabilityParam->value().toInt();
        if(index < dimmers.length())
        {
            static_cast<DimmerCapability*>(dimmers[index])->setPercent(m_intensityParam->value().toDouble() * m_blendParam->value().toDouble(), context->dmxMatrix, context->strength);
        }
    }

}

} // namespace photon
