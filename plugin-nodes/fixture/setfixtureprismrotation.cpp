#include "setfixtureprismrotation.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/capability/prismrotationcapability.h"
#include "fixture/fixture.h"

namespace photon {

keira::NodeInformation SetFixturePrismRotation::info()
{
    keira::NodeInformation toReturn([](){return new SetFixturePrismRotation;});
    toReturn.name = "Set Fixture Prism Rotation";
    toReturn.nodeId = "photon.plugin.node.set-fixture-prism-rotation";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

SetFixturePrismRotation::SetFixturePrismRotation() : keira::Node("photon.plugin.node.set-fixture-prism-rotation"){}

void SetFixturePrismRotation::createParameters()
{

    m_modeParam = new keira::OptionParameter("mode","Mode",{"Continuous","Indexed"},0);
    addParameter(m_modeParam);

    m_angleParam = new keira::DecimalParameter("amount","Amount", 0);
    addParameter(m_angleParam);

    m_usePrismParam = new keira::BooleanParameter("usePrism","Use Prism", true);
    addParameter(m_usePrismParam);

}

void SetFixturePrismRotation::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        float rotationAmount = m_angleParam->value().toFloat();

        bool useAngle = m_modeParam->value().toInt() == 1;
        bool usePrism = m_usePrismParam->value().toBool();

        if(!usePrism)
            return;

        auto allCapabilities = context->fixture->findCapability(CapabilityType::Capability_PrismRotation);

        for(auto capability : allCapabilities)
        {
            auto prismCapability = static_cast<PrismRotationCapability*>(capability);

            if(prismCapability->supportsAngle() && useAngle)
            {
                prismCapability->setAngleDegrees(rotationAmount,context->dmxMatrix,1.0);
                return;
            }
            else
            {
                if(rotationAmount > 0 && prismCapability->maxSpeed() > 0)
                {
                    prismCapability->setSpeed(rotationAmount,context->dmxMatrix,1.0);
                    return;
                }
                else if(rotationAmount < 0 && prismCapability->maxSpeed() < 0)
                {
                    prismCapability->setSpeed(rotationAmount,context->dmxMatrix,1.0);
                    return;
                }
                else if(rotationAmount == 0 && prismCapability->isStop() && usePrism)
                {
                    prismCapability->selectCapability(context->dmxMatrix);
                    return;
                }
            }
        }

    }

}

} // namespace photon
