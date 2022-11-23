#include "globalsnode.h"
#include "model/parameter/decimalparameter.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

const QByteArray GlobalsNode::RelativeTime = "relativeTime";
const QByteArray GlobalsNode::GlobalTime = "globalTime";
const QByteArray GlobalsNode::DelayTime = "delayTime";
const QByteArray GlobalsNode::Strength = "strength";

class GlobalsNode::Impl
{
public:
    keira::DecimalParameter *relativeParam;
    keira::DecimalParameter *globalParam;
    keira::DecimalParameter *delayParam;
    keira::DecimalParameter *strengthParam;
};


keira::NodeInformation GlobalsNode::info()
{
    keira::NodeInformation toReturn([](){return new GlobalsNode;});
    toReturn.name = "Globals";
    toReturn.nodeId = "photon.routine.globals";

    return toReturn;
}

GlobalsNode::GlobalsNode() : keira::Node("photon.routine.globals"),m_impl(new Impl)
{
    setName("Globals");
}

GlobalsNode::~GlobalsNode()
{
    delete m_impl;
}

void GlobalsNode::createParameters()
{
    m_impl->relativeParam = new keira::DecimalParameter(RelativeTime,"Relative Time", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->relativeParam);
    m_impl->globalParam = new keira::DecimalParameter(GlobalTime,"Global Time", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->globalParam);
    m_impl->delayParam = new keira::DecimalParameter(DelayTime,"Delay Time", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->delayParam);
    m_impl->strengthParam = new keira::DecimalParameter(Strength,"Strength", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->strengthParam);
}

void GlobalsNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    m_impl->relativeParam->setValue(context->relativeTime);
    m_impl->globalParam->setValue(context->globalTime);
    m_impl->delayParam->setValue(context->delayTime);
    m_impl->strengthParam->setValue(context->strength);

    //qDebug() << context->strength << context->relativeTime;
}

} // namespace photon
