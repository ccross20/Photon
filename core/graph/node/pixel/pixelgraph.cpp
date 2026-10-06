#include <QJsonArray>
#include <QJsonObject>
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
    // "routine" covers routines and clip graphs (FixtureClip's content graph).
    toReturn.graphs = QByteArrayList{"bus","surface","routine"};

    return toReturn;
}


PixelGraph::PixelGraph() : keira::SubGraphNode("photon.node.pixel-graph") {
    setName("Pixel Graph");

    m_globalsNode = new GraphContextNode;
    m_globalsNode->configure(GraphContextNode::pixelPorts());
    graph()->addNode(m_globalsNode);

    // Seed with a Set Pixel Color node - the whole point of a pixel graph is
    // to drive pixel color, so an empty graph is never actually useful as-is.
    // Created by id through the plugin factory rather than a direct #include:
    // Set Pixel Color lives in plugin-nodes, which depends on core, not the
    // other way around.
    m_seedSetColorNode = photonApp->plugins()->nodeLibrary()->createNode("photon.plugin.node.set-pixel-color");
    if(m_seedSetColorNode)
    {
        m_seedSetColorNode->setPosition(m_globalsNode->position() + QPointF(500, 0));
        graph()->addNode(m_seedSetColorNode);
    }

    graph()->drainCommandQueue(); // apply immediately so the queued addNode never
                                  // outlives m_globalsNode (see readFromJson below)
    graph()->setName("Pixel Graph");
    graph()->setGraphTypeId("pixel");
}

PixelGraph::~PixelGraph()
{
}


void PixelGraph::createParameters()
{
    m_pixelsParam = new PixelListParameter(PixelGraph::Pixels,"Pixels", QVector<PixelParameterData>()
                                               , keira::AllowMultipleOutput | keira::AllowSingleInput);
    addParameter(m_pixelsParam);

    m_enabledParam = new keira::BooleanParameter(Enabled, "Enabled", true);
    addParameter(m_enabledParam);

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
    if(m_seedSetColorNode)
        graph()->removeNode(m_seedSetColorNode);
    graph()->drainCommandQueue(); // apply the removal (and any still-pending addNode
                                  // from construction) BEFORE freeing the pointer, so
                                  // no queued command is left referencing freed memory
    delete m_globalsNode;
    delete m_seedSetColorNode;
    m_seedSetColorNode = nullptr;

    // "useTimeMachine" was a parameter of the removed DMX time machine. Drop
    // it from older saves - keira recreates any saved parameter it doesn't
    // know, which would bring back a "Use Time Machine" field that does
    // nothing.
    QJsonObject json = t_json;
    QJsonArray parameters;
    for(const auto &param : t_json.value("parameters").toArray())
    {
        if(param.toObject().value("id").toString() != QLatin1String("useTimeMachine"))
            parameters.append(param);
    }
    json.insert("parameters", parameters);

    keira::SubGraphNode::readFromJson(json, t_library);

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
    // Evaluated on a copy: the per-pixel fixture/index set below
    // must not leak back to the caller. In a clip graph a node evaluated after
    // this one would otherwise inherit the last pixel's fixture - and a
    // Fixture State that sees a fixture on its context switches to
    // per-fixture mode and writes only that one.
    RoutineEvaluationContext local(*static_cast<RoutineEvaluationContext*>(t_context));
    auto *context = &local;

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
            // relativeTime is left as the caller set it: the clip's own time
            // inside a clip, the same as globalTime in a bus or surface graph.
            context->timeOffset = 0;
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
}

} // namespace photon
