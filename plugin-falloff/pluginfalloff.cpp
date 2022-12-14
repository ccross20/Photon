#include "pluginfalloff.h"
#include "plugin/pluginfactory.h"
#include "randomfalloff.h"

//inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

bool PluginFalloff::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)


    photonApp->plugins()->registerFalloffEffect(RandomFalloff::info());

    return true;
}

QVersionNumber PluginFalloff::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginFalloff::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginFalloff::name()
{
    return "Falloff Effects";
}

QString PluginFalloff::description()
{
    return "Adds Falloff Effects";
}

QString PluginFalloff::id()
{
    return "photon.falloff-effects";
}


} // photon
