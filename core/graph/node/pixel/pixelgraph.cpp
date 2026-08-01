#include "pixelgraph.h"
#include "graph/node/graphcontextnode.h"
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

    m_globalsNode = new GraphContextNode;
    m_globalsNode->configure(GraphContextNode::pixelPorts());
    graph()->addNode(m_globalsNode);
    graph()->drainCommandQueue(); // apply immediately so the queued addNode never
                                  // outlives m_globalsNode (see readFromJson below)
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

keira::NodeLibrary *PixelGraph::nodeLibrary() const
{
    return photonApp->plugins()->nodeLibrary();
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
    graph()->drainCommandQueue(); // apply the removal (and any still-pending addNode
                                  // from construction) BEFORE freeing the pointer, so
                                  // no queued command is left referencing freed memory
    delete m_globalsNode;


    keira::SubGraphNode::readFromJson(t_json, t_library);

    m_globalsNode = dynamic_cast<GraphContextNode*>(graph()->findNode("Globals"));
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

    // Total pixel count is constant across the loop; the context node fills the
    // per-frame/per-pixel context ports (time/fixture/index) itself during eval.
    m_globalsNode->setValue(GraphContextNode::PixelTotalPort, pixels.length());

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
            context->fixtureIndex = fixtureCounter;
            context->relativeTime = context->globalTime;
            context->timeOffset = 0;
            if(useTimeMachine)
                m_timeMachine->setTargetFrame(context->frame);
            // Pixel-specific ports aren't carried on the eval context — set them
            // directly; the context node fills fixture/index/time from the context.
            m_globalsNode->setValue(GraphContextNode::PixelIndexPort, pixel.index);
            m_globalsNode->setValue(GraphContextNode::PixelGlobalIndexPort, index);

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
