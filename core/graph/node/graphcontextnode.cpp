#include "graphcontextnode.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixturelistparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

const QByteArray GraphContextNode::DMXMatrixPort      = "dmxMatrix";
const QByteArray GraphContextNode::FixtureListPort    = "fixtureList";
const QByteArray GraphContextNode::FixturePort        = "fixture";
const QByteArray GraphContextNode::FixtureIndexPort   = "fixtureIndex";
const QByteArray GraphContextNode::FixtureTotalPort   = "fixtureTotal";
const QByteArray GraphContextNode::RelativeTimePort   = "relativeTime";
const QByteArray GraphContextNode::GlobalTimePort     = "globalTime";
const QByteArray GraphContextNode::TimeOffsetPort     = "timeOffset";
const QByteArray GraphContextNode::WidthPort          = "width";
const QByteArray GraphContextNode::HeightPort         = "height";
const QByteArray GraphContextNode::PixelIndexPort     = "pixelIndex";
const QByteArray GraphContextNode::PixelGlobalIndexPort = "pixelGlobalIndex";
const QByteArray GraphContextNode::PixelTotalPort     = "pixelTotal";

keira::NodeInformation GraphContextNode::info()
{
    keira::NodeInformation toReturn([](){return new GraphContextNode;});
    toReturn.name = "Globals";
    toReturn.nodeId = "photon.graph.context";
    // Auto-created by subgraphs; scope to a graph type nothing uses so it never
    // shows in the Add Node menu (which lists nodes whose graphs list is empty or
    // contains the current graph type).
    toReturn.graphs = QByteArrayList{"__internal"};
    return toReturn;
}

GraphContextNode::GraphContextNode() : keira::Node("photon.graph.context")
{
    setName("Globals");
    // Always re-publish context each frame so inner nodes see fresh values (the
    // old FixtureGlobalsNode relied on this for per-fixture evaluation).
    setIsAlwaysDirty(true);
}

QByteArrayList GraphContextNode::fixturePorts()
{
    return {DMXMatrixPort, FixtureListPort, FixturePort, FixtureIndexPort,
            FixtureTotalPort, RelativeTimePort, GlobalTimePort, TimeOffsetPort};
}

QByteArrayList GraphContextNode::canvasPorts()
{
    return {WidthPort, HeightPort, RelativeTimePort, GlobalTimePort};
}

QByteArrayList GraphContextNode::pixelPorts()
{
    return {FixturePort, FixtureIndexPort, PixelIndexPort, PixelGlobalIndexPort,
            PixelTotalPort, RelativeTimePort, GlobalTimePort, TimeOffsetPort};
}

keira::Parameter *GraphContextNode::addPort(const QByteArray &t_id)
{
    keira::Parameter *param = nullptr;

    if(t_id == DMXMatrixPort)
        param = new DMXMatrixParameter(t_id, "DMX Matrix", DMXMatrix(), keira::AllowMultipleOutput);
    else if(t_id == FixtureListPort)
        param = new FixtureListParameter(t_id, "All Fixtures", QVector<FixtureParameterData>(), keira::AllowMultipleOutput);
    else if(t_id == FixturePort)
        param = new FixtureParameter(t_id, "Fixture", "", keira::AllowMultipleOutput);
    else if(t_id == FixtureIndexPort)
        param = new keira::IntegerParameter(t_id, "Fixture Index", 0, keira::AllowMultipleOutput);
    else if(t_id == FixtureTotalPort)
        param = new keira::IntegerParameter(t_id, "Total Fixtures", 0, keira::AllowMultipleOutput);
    else if(t_id == RelativeTimePort)
        param = new keira::DecimalParameter(t_id, "Relative Time", 0, keira::AllowMultipleOutput);
    else if(t_id == GlobalTimePort)
        param = new keira::DecimalParameter(t_id, "Global Time", 0, keira::AllowMultipleOutput);
    else if(t_id == TimeOffsetPort)
        param = new keira::DecimalParameter(t_id, "Time Offset", 0, keira::AllowMultipleOutput);
    else if(t_id == WidthPort)
        param = new keira::IntegerParameter(t_id, "Width", 256, keira::AllowMultipleOutput);
    else if(t_id == HeightPort)
        param = new keira::IntegerParameter(t_id, "Height", 256, keira::AllowMultipleOutput);
    else if(t_id == PixelIndexPort)
        param = new keira::IntegerParameter(t_id, "Pixel Index", 0, keira::AllowMultipleOutput);
    else if(t_id == PixelGlobalIndexPort)
        param = new keira::IntegerParameter(t_id, "Pixel Global Index", 0, keira::AllowMultipleOutput);
    else if(t_id == PixelTotalPort)
        param = new keira::IntegerParameter(t_id, "Total Pixels", 0, keira::AllowMultipleOutput);

    if(param)
        addParameter(param);
    return param;
}

void GraphContextNode::configure(const QByteArrayList &t_portIds)
{
    // Idempotent: ports restored by readFromJson are left as they are (keeping
    // their existing connections), and only genuinely missing ones are added.
    // That lets the enclosing subgraph re-run this after loading so a graph
    // saved before a port existed gains it, instead of being permanently
    // stuck with the port set it was first created with.
    for(const QByteArray &id : t_portIds)
        if(!findParameter(id))
            addPort(id);
}

void GraphContextNode::createParameters()
{
    // Ports are added via configure() (fresh) or restored by readFromJson (load).
}

void GraphContextNode::evaluate(keira::EvaluationContext *t_context) const
{
    // Guard against a plain (non-routine) context: e.g. a canvas graph opened
    // directly in a node editor gets live-ticked by a generic GraphEvaluator
    // whose default context factory produces a bare keira::EvaluationContext,
    // not a RoutineEvaluationContext (see CanvasOutputNode::evaluate(), which
    // already guards the same way).
    if(auto *context = dynamic_cast<RoutineEvaluationContext*>(t_context))
        fillFromContext(*context);
}

void GraphContextNode::fillFromContext(const RoutineEvaluationContext &t_context) const
{
    for(keira::Parameter *param : parameters())
    {
        const QByteArray id = param->id();
        if(id == RelativeTimePort)
            param->setValue(t_context.relativeTime);
        else if(id == GlobalTimePort)
            param->setValue(t_context.globalTime);
        else if(id == TimeOffsetPort)
            param->setValue(t_context.timeOffset);
        else if(id == FixturePort)
            param->setValue(t_context.fixture ? t_context.fixture->uniqueId() : QByteArray{});
        else if(id == FixtureIndexPort)
            param->setValue(t_context.fixtureIndex);
        else if(id == WidthPort)
            param->setValue(t_context.canvasResolution.width());
        else if(id == HeightPort)
            param->setValue(t_context.canvasResolution.height());
        // DMXMatrix / FixtureList / pixel ports are pushed by the subgraph.
    }
}

} // namespace photon
