#include "plugineffects.h"
#include "plugin/pluginfactory.h"
#include "generator/noiseeffect.h"
#include "generator/sineeffect.h"
#include "generator/pulseeffect.h"
#include "generator/sawtootheffect.h"
#include "time/stuttereffect.h"
#include "time/loopeffect.h"
#include "modifier/smootheffect.h"
#include "modifier/easeeffect.h"
#include "mask/treemaskeffect.h"
#include "mask/randommaskeffect.h"

//inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

bool PluginEffects::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)

    photonApp->plugins()->registerChannelEffect(PulseEffect::info());
    photonApp->plugins()->registerChannelEffect(SineEffect::info());
    photonApp->plugins()->registerChannelEffect(SawtoothEffect::info());
    photonApp->plugins()->registerChannelEffect(StutterEffect::info());
    photonApp->plugins()->registerChannelEffect(LoopEffect::info());
    photonApp->plugins()->registerChannelEffect(SmoothEffect::info());
    photonApp->plugins()->registerChannelEffect(EaseEffect::info());
    photonApp->plugins()->registerChannelEffect(NoiseEffect::info());

    photonApp->plugins()->registerMaskEffect(TreeMaskEffect::info());
    photonApp->plugins()->registerMaskEffect(RandomMaskEffect::info());

    return true;
}

QVersionNumber PluginEffects::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginEffects::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginEffects::name()
{
    return "Channel Effects";
}

QString PluginEffects::description()
{
    return "Adds Channel Effects";
}

QString PluginEffects::id()
{
    return "photon.channel-effects";
}


} // photon
