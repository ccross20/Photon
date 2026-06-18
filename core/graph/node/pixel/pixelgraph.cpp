#include "pixelgraph.h"
#include "pixelglobalsnode.h"
#include "model/graph.h"
#include "graph/parameter/pixellistparameter.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "plugin/pluginfactory.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

const QByteArray PixelGraph::Pixels = "pixels";
const QByteArray PixelGraph::Enabled = "enabled";
const QByteArray PixelGraph::PixelSubGraphId = "pixel";


keira::NodeInformation PixelGraph::info()
{
    keira::NodeInformation toReturn([](){return new PixelGraph;});
    toReturn.name = "Pixel Graph";
    toReturn.nodeId = "photon.node.pixel-graph";
    toReturn.categories = {"Pixel"};
    toReturn.graphs = QByteArrayList{"bus","surface"};

    return toReturn;
}


PixelGraph::PixelGraph() : keira::SubGraphNode("photon.node.pixel-graph") {
    setName("Pixel Graph");

    m_globalsNode = new PixelGlobalsNode;
    m_globalsNode->createParameters();
    graph()->addNode(m_globalsNode);
    graph()->setName("Pixel Graph");
    graph()->setGraphTypeId("pixel");

    m_timeMachine = new DMXTimeMachine;
}

PixelGraph::~PixelGraph()
{
    delete m_timeMachine;
}


void PixelGraph::createParameters()
{
    m_pixelsParam = new PixelListParameter(PixelGraph::Pixels,"Pixels", QVector<PixelParameterData>()
                                               , keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_pixelsParam);

    m_enabledParam = new keira::BooleanParameter(Enabled, "Enabled", true);
    addParameter(m_enabledParam);

    m_useTimeMachineParam = new keira::BooleanParameter("useTimeMachine","Use Time Machine",false);
    addParameter(m_useTimeMachineParam);

    m_priortyParam = new keira::IntegerParameter("priority","Priority",0);
    addParameter(m_priortyParam);
}

void PixelGraph::parameterWasAdded(keira::Parameter *t_param)
{
    if(m_pixelsParam == t_param || m_enabledParam == t_param || t_param == m_priortyParam || t_param == m_useTimeMachineParam)
        return;

    auto clonedParam = t_param->clone(photonApp->plugins()->nodeLibrary());
    clonedParam->setConnectionOptions(keira::AllowMultipleOutput);
    m_globalsNode->addParameter(clonedParam);
    m_passThroughParams.append(clonedParam);
    m_globalsParams.append(t_param);
}

void PixelGraph::parameterWasRemoved(keira::Parameter *t_param)
{

}

void PixelGraph::parameterWasModified(keira::Parameter *t_param)
{
    if(t_param == m_priortyParam)
    {
        qDebug() << "priority changed";
        setPriority(m_priortyParam->value().toInt());
        markDirty(keira::Dirty_Priority);
    }
}

void PixelGraph::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    graph()->removeNode(m_globalsNode);
    delete m_globalsNode;


    keira::SubGraphNode::readFromJson(t_json, t_library);

    m_globalsNode = dynamic_cast<PixelGlobalsNode*>(graph()->findNode("Globals"));

    for(auto param : parameters())
    {
        if(m_pixelsParam == param || param == m_priortyParam || param == m_useTimeMachineParam)
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

void PixelGraph::prepForEvaluation()
{
    Node::prepForEvaluation();

    graph()->prepForEvaluation();
}


void PixelGraph::evaluate(keira::EvaluationContext *t_context) const
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

    auto pixels = m_pixelsParam->value().value<QVector<PixelParameterData>>();

    //m_globalsNode->setValue(FixtureGlobalsNode::DMXParam, context->dmxMatrix);
    m_globalsNode->setValue(PixelGlobalsNode::TimeParam, context->globalTime);
    m_globalsNode->setValue(PixelGlobalsNode::PixelTotalParam, pixels.length());
    m_globalsNode->setValue(PixelGlobalsNode::GlobalTimeParam, context->globalTime);

    for(int i = 0; i < m_passThroughParams.length(); ++i)
    {
        m_passThroughParams[i]->setValue(m_globalsParams[i]->value());
    }


    Fixture *lastFixture = nullptr;
    int index = 0;
    int fixtureCounter = -1;
    Fixture *fix = nullptr;
    for(const auto &pixel : pixels)
    {
        if(lastFixture && lastFixture->uniqueId() == pixel.fixtureId)
            fix = lastFixture;
        else
        {
            fix = photonApp->project()->fixtures()->fixtureWithId(pixel.fixtureId);
            fixtureCounter++;
        }
        if(fix)
        {
            lastFixture = fix;
            context->fixture = fix;
            context->relativeTime = context->globalTime;
            if(useTimeMachine)
                m_timeMachine->setTargetFrame(context->frame);
            m_globalsNode->setValue(PixelGlobalsNode::FixtureParam, pixel.fixtureId);
            m_globalsNode->setValue(PixelGlobalsNode::FixtureIndexParam, fixtureCounter);
            m_globalsNode->setValue(PixelGlobalsNode::PixelIndexParam, pixel.index);
            m_globalsNode->setValue(PixelGlobalsNode::PixelGlobalIndexParam, index);
            m_globalsNode->setValue(PixelGlobalsNode::TimeOffsetParam, 0);
            m_globalsNode->setValue(PixelGlobalsNode::TimeParam, context->globalTime );

            //qDebug() << "Eval" << pixel.index;
            SubGraphNode::evaluate(context);
            index++;
            fix = nullptr;
        }
        else
        {
            qDebug() << "Could not find fixture";
        }
    }


    m_timeMachine->releaseMatrix();



}

} // namespace photon
