#include "pluginnodes.h"
#include "plugin/pluginfactory.h"
#include "fixture/setfixturepan.h"
#include "fixture/setfixturetilt.h"
#include "fixture/setfixturefocus.h"
#include "fixture/setfixturezoom.h"
#include "fixture/setfixturedimmer.h"
#include "fixture/setfixturestrobe.h"
#include "fixture/lookattarget.h"
#include "canvas/canvasreader.h"
#include "canvas/canvaswriter.h"
#include "canvas/drawrectangle.h"
#include "math/circlenode.h"
#include "math/noisenode.h"

//inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

bool PluginNodes::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)


    photonApp->plugins()->registerNode(SetFixturePan::info());
    photonApp->plugins()->registerNode(SetFixtureTilt::info());
    photonApp->plugins()->registerNode(SetFixtureFocus::info());
    photonApp->plugins()->registerNode(SetFixtureZoom::info());
    photonApp->plugins()->registerNode(SetFixtureDimmer::info());
    photonApp->plugins()->registerNode(SetFixtureStrobe::info());
    photonApp->plugins()->registerNode(CanvasReader::info());
    photonApp->plugins()->registerNode(CanvasWriter::info());
    photonApp->plugins()->registerNode(DrawRectangle::info());
    photonApp->plugins()->registerNode(LookAtTarget::info());
    photonApp->plugins()->registerNode(CircleNode::info());
    photonApp->plugins()->registerNode(NoiseNode::info());

    return true;
}

QVersionNumber PluginNodes::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginNodes::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginNodes::name()
{
    return "Nodes";
}

QString PluginNodes::description()
{
    return "Adds Graph Nodes";
}

QString PluginNodes::id()
{
    return "photon.nodes";
}


} // photon
