#include <QNetworkInterface>
#include "pluginoutputnodes.h"
#include "plugin/pluginfactory.h"
#include "sacntransmitnode.h"

namespace photon {

bool PluginOutputNodes::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)




    photonApp->plugins()->registerNode(SACNTransmitNode::info());

    return true;
}

QVersionNumber PluginOutputNodes::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginOutputNodes::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginOutputNodes::name()
{
    return "Output Nodes";
}

QString PluginOutputNodes::description()
{
    return "Adds nodes for outputting DMX data";
}

QString PluginOutputNodes::id()
{
    return "photon.clip-effects";
}


} // photon
