#include "pluginnodes.h"
#include "plugin/pluginfactory.h"
#include "fixture/setfixturepan.h"
#include "fixture/setfixturetilt.h"
#include "canvas/canvasreader.h"
#include "canvas/canvaswriter.h"
#include "canvas/drawrectangle.h"

//inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

bool PluginNodes::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)


    photonApp->plugins()->registerNode(SetFixturePan::info());
    photonApp->plugins()->registerNode(SetFixtureTilt::info());
    photonApp->plugins()->registerNode(CanvasReader::info());
    photonApp->plugins()->registerNode(CanvasWriter::info());
    photonApp->plugins()->registerNode(DrawRectangle::info());

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
