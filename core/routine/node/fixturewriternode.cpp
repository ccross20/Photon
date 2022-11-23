#include "fixturewriternode.h"
#include "model/parameter/decimalparameter.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"

namespace photon {

const QByteArray FixtureWriterNode::BrightnessInput = "brightnessInput";
const QByteArray FixtureWriterNode::ColorInput = "colorInput";
const QByteArray FixtureWriterNode::TiltInput = "tiltInput";

class FixtureWriterNode::Impl
{
public:
    keira::DecimalParameter *brightnessParam;
    keira::DecimalParameter *colorParam;
    keira::DecimalParameter *tiltParam;
};


keira::NodeInformation FixtureWriterNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureWriterNode;});
    toReturn.name = "Fixture Writer";
    toReturn.nodeId = "photon.routine.fixture-writer";

    return toReturn;
}

FixtureWriterNode::FixtureWriterNode() : keira::Node("photon.routine.fixture-writer"),m_impl(new Impl)
{
    setName("Fixture Writer");
}

FixtureWriterNode::~FixtureWriterNode()
{
    delete m_impl;
}

void FixtureWriterNode::createParameters()
{
    m_impl->brightnessParam = new keira::DecimalParameter(BrightnessInput,"Brightness", 0.0);
    m_impl->brightnessParam->setMinimum(0.0);
    m_impl->brightnessParam->setMaximum(1.0);
    addParameter(m_impl->brightnessParam);
}

void FixtureWriterNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        context->fixture->setBrightness(m_impl->brightnessParam->value().toDouble() * context->strength, context->dmxMatrix);
    }

}

} // namespace photon
