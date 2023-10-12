#include "pluginclipeffects.h"
#include "plugin/pluginfactory.h"

#include "pan-and-tilt/tracepathclip.h"

#include "canvas/draw/drawellipseclipeffect.h"
#include "canvas/draw/drawrectangleclipeffect.h"
#include "canvas/fill/fillnoiseeffect.h"

#include "strobe/beatstrobeeffect.h"

namespace photon {

bool PluginClipEffects::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)

    photonApp->plugins()->registerClipEffect(TracePathClip::info());
    photonApp->plugins()->registerClipEffect(DrawEllipseClipEffect::info());
    photonApp->plugins()->registerClipEffect(DrawRectangleClipEffect::info());
    photonApp->plugins()->registerClipEffect(FillNoiseEffect::info());
    photonApp->plugins()->registerClipEffect(BeatStrobeEffect::info());

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
