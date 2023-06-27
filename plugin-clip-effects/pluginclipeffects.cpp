#include "pluginclipeffects.h"
#include "plugin/pluginfactory.h"

#include "pan-and-tilt/tracepathclip.h"


namespace photon {

bool PluginClipEffects::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)

    photonApp->plugins()->registerClipEffect(TracePathClip::info());

    return true;
}

QVersionNumber PluginClipEffects::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginClipEffects::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginClipEffects::name()
{
    return "Clip Effects";
}

QString PluginClipEffects::description()
{
    return "Adds Clip Effects";
}

QString PluginClipEffects::id()
{
    return "photon.clip-effects";
}


} // photon
