#include "fixturesubgraphnode.h"
#include "fixtureglobalsnode.h"
#include "model/graph.h"
#include "surface/surfacegraph.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "plugin/pluginfactory.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

const QByteArray FixtureSubGraphNode::Fixtures = "fixtures";
const QByteArray FixtureSubGraphNode::Enabled = "enabled";
const QByteArray FixtureSubGraphNode::FixtureSubGraphId = "fixture";

keira::NodeInformation FixtureSubGraphNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureSubGraphNode;});
    toReturn.name = "Fixture Graph";
    toReturn.nodeId = "photon.node.fixture-graph";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"bus","surface"};

    return toReturn;
}

FixtureSubGraphNode::FixtureSubGraphNode() : keira::SubGraphNode("photon.node.fixture-graph") {
    setName("Fixture Graph");

    m_globalsNode = new FixtureGlobalsNode;
    m_globalsNode->createParameters();
    graph()->addNode(m_globalsNode);
    graph()->setName("Fixture Graph");
    graph()->setGraphTypeId("fixture");

    m_timeMachine = new DMXTimeMachine;
}

FixtureSubGraphNode::~FixtureSubGraphNode()
{
    delete m_timeMachine;
}

void FixtureSubGraphNode::createParameters()
{
    m_fixturesParam = new FixtureListParameter(FixtureSubGraphNode::Fixtures,"Fixtures", QVector<FixtureParameterData>()
                                                , keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_fixturesParam);

    m_enabledParam = new keira::BooleanParameter(Enabled, "Enabled", true);
    addParameter(m_enabledParam);

    m_useTimeMachineParam = new keira::BooleanParameter("useTimeMachine","Use Time Machine",false);
    addParameter(m_useTimeMachineParam);

    m_priortyParam = new keira::IntegerParameter("priority","Priority",0);
    addParameter(m_priortyParam);
}

void FixtureSubGraphNode::parameterWasAdded(keira::Parameter *t_param)
{
    if(m_fixturesParam == t_param || m_enabledParam == t_param || t_param == m_priortyParam || t_param == m_useTimeMachineParam)
        return;

    auto clonedParam = t_param->clone(photonApp->plugins()->nodeLibrary());
    clonedParam->setConnectionOptions(keira::AllowMultipleOutput);
    m_globalsNode->addParameter(clonedParam);
    m_passThroughParams.append(clonedParam);
    m_globalsParams.append(t_param);
}

void FixtureSubGraphNode::parameterWasRemoved(keira::Parameter *t_param)
{

}

void FixtureSubGraphNode::parameterWasModified(keira::Parameter *t_param)
{
    if(t_param == m_priortyParam)
    {
        setPriority(m_priortyParam->value().toInt());
        markDirty(keira::Dirty_Priority);
    }
}

void FixtureSubGraphNode::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    graph()->removeNode(m_globalsNode);
    delete m_globalsNode;


    keira::SubGraphNode::readFromJson(t_json, t_library);

    m_globalsNode = dynamic_cast<FixtureGlobalsNode*>(graph()->findNode("Globals"));

    for(auto param : parameters())
    {
        if(m_fixturesParam == param || param == m_priortyParam || param == m_useTimeMachineParam)
            continue;

        auto nodeParam = m_globalsNode->findParameter(param->id());
        if(nodeParam)
        {
            m_globalsParams.append(param);
            m_passThroughParams.append(nodeParam);
        }
        else
            qWarning() << "Could not relink parameter: " << param->id();

    }
}

void FixtureSubGraphNode::prepForEvaluation()
{
    Node::prepForEvaluation();

    graph()->prepForEvaluation();
}


void FixtureSubGraphNode::evaluate(keira::EvaluationContext *t_context) const
{
    if(!m_enabledParam->value().toBool())
        return;

    //qDebug() << name();
    auto context = static_cast<RoutineEvaluationContext*>(t_context);
    context->timeMachine = m_timeMachine;
    m_timeMachine->initializeMatrix(&context->dmxMatrix, context->frame);
    m_timeMachine->writeStoredData();
    m_timeMachine->setTargetFrame(context->frame);

    bool useTimeMachine = m_useTimeMachineParam->value().toBool();

    auto fixtures = m_fixturesParam->value().value<QVector<FixtureParameterData>>();

    //m_globalsNode->setValue(FixtureGlobalsNode::DMXParam, context->dmxMatrix);
    m_globalsNode->setValue(FixtureGlobalsNode::FixtureListParam, m_fixturesParam->value());
    m_globalsNode->setValue(FixtureGlobalsNode::TimeParam, context->globalTime);

    for(int i = 0; i < m_passThroughParams.length(); ++i)
    {
        m_passThroughParams[i]->setValue(m_globalsParams[i]->value());
    }

    /*
    if(!m_fixturesParam->hasInput())
    {



        m_globalsNode->setValue(FixtureGlobalsNode::TimeOffsetParam, 0);
        m_globalsNode->setValue(FixtureGlobalsNode::TimeParam, context->globalTime);
        for(auto fixture : photonApp->project()->fixtures()->fixtures())
        {

            context->fixture = fixture;
            context->relativeTime = context->globalTime;
            m_globalsNode->setValue(FixtureGlobalsNode::FixtureParam, fixture->uniqueId());
            SubGraphNode::evaluate(context);
        }
    }
    else
*/
    {
        int index = 0;
        for(const auto &fixture : fixtures)
        {
            auto fix = photonApp->project()->fixtures()->fixtureWithId(fixture.fixtureId);
            if(fix)
            {
                context->fixture = fix;
                context->relativeTime = context->globalTime + fixture.offset;
                if(useTimeMachine)
                    m_timeMachine->setTargetFrame(context->frame + std::floor((fixture.offset * 1000.0)/30.0));
                m_globalsNode->setValue(FixtureGlobalsNode::FixtureParam, fixture.fixtureId);
                m_globalsNode->setValue(FixtureGlobalsNode::FixtureIndexParam, index);
                m_globalsNode->setValue(FixtureGlobalsNode::TimeOffsetParam, fixture.offset);
                m_globalsNode->setValue(FixtureGlobalsNode::TimeParam, context->globalTime + fixture.offset);
                m_globalsNode->setValue(FixtureGlobalsNode::GlobalTimeParam, context->globalTime);
                SubGraphNode::evaluate(context);
                index++;
            }
            else
            {
                qDebug() << "Could not find fixture";
            }
        }
    }

    m_timeMachine->releaseMatrix();



}

} // namespace photon
