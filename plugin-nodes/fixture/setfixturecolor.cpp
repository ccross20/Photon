#include "setfixturecolor.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"

namespace photon {

keira::NodeInformation SetFixtureColor::info()
{
    keira::NodeInformation toReturn([](){return new SetFixtureColor;});
    toReturn.name = "Set Fixture Color";
    toReturn.nodeId = "photon.plugin.node.set-fixture-color";
    toReturn.categories = {"Fixture"};

    return toReturn;
}

SetFixtureColor::SetFixtureColor() : keira::Node("photon.plugin.node.set-fixture-color")
{

}

void SetFixtureColor::createParameters()
{
    m_colorParam = new ColorParameter("colorInput","Color", Qt::white);
    addParameter(m_colorParam);

    m_capabilityParam = new keira::IntegerParameter("capabilityIndex","Capability", 0);
    m_capabilityParam->setMinimum(0);
    m_capabilityParam->setMaximum(100);
    addParameter(m_capabilityParam);

    m_blendParam = new keira::DecimalParameter("blendInput","Blend", 1.0);
    m_blendParam->setMinimum(0.0);
    m_blendParam->setMaximum(1.0);
    addParameter(m_blendParam);
}

void SetFixtureColor::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        auto pans = context->fixture->findCapability(Capability_Color);

        int index = m_capabilityParam->value().toInt();
        if(index < pans.length())
        {
            static_cast<ColorCapability*>(pans[index])->setColor(m_colorParam->value().value<QColor>(), context->dmxMatrix, context->strength * m_blendParam->value().toDouble());
        }
    }

}

} // namespace photon
