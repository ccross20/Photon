#include "pluginnodes.h"
#include "plugin/pluginfactory.h"
#include "fixture/setfixturecolor.h"
#include "fixture/setfixturepan.h"
#include "fixture/setfixturetilt.h"
#include "fixture/setfixturefocus.h"
#include "fixture/setfixturezoom.h"
#include "fixture/setfixturedimmer.h"
#include "fixture/setfixtureslot.h"
#include "fixture/setfixturestrobe.h"
#include "fixture/setfixtureprismrotation.h"
#include "fixture/setfixtureslotrotation.h"
#include "fixture/lookattarget.h"
#include "fixture/lookindirection.h"
#include "fixture/mirrorpantilt.h"
#include "fixture/scatterbeams.h"
#include "fixture-list/fixturelistrandomsubsetnode.h"
#include "fixture-list/fixturelistintervalsubsetnode.h"
#include "fixture-list/fixturelistmergenode.h"
#include "canvas/canvasreader.h"
#include "canvas/canvaswriter.h"
#include "canvas/drawrectangle.h"
#include "canvas/drawellipse.h"
#include "canvas/fadecanvas.h"
#include "canvas/renderpath.h"
#include "canvas/renderstroke.h"
#include "canvas/masknode.h"
#include "canvas/transform/transformtexturenode.h"
#include "color/colorfromhsv.h"
#include "color/randomcolornode.h"
#include "color/sparklenode.h"
#include "color-palette/colorpalettenode.h"
#include "color-palette/colorfromcolorpalette.h"
#include "color-palette/fadecolorpalettenode.h"
#include "color-palette/colortheorypalettenode.h"
#include "math/circlenode.h"
#include "math/noisenode.h"
#include "math/randomnumbernode.h"
#include "math/stopwatchnode.h"
#include "math/remapvaluenode.h"
#include "math/ifnode.h"
#include "animation/timer.h"
#include "animation/pulsenode.h"
#include "animation/oscillatornode.h"
#include "animation/timenode.h"
#include "animation/beatreducernode.h"
#include "falloff/linearfalloffnode.h"
#include "falloff/spatialfalloffnode.h"
#include "falloff/randomfalloffnode.h"
#include "pixel/pixelsfromfixturelist.h"
#include "pixel/setpixelcolor.h"
#include "utils/djconnectornode.h"

//inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

bool PluginNodes::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)


    photonApp->plugins()->registerNode(SetFixtureColor::info());
    photonApp->plugins()->registerNode(SetFixturePan::info());
    photonApp->plugins()->registerNode(SetFixtureTilt::info());
    photonApp->plugins()->registerNode(SetFixtureFocus::info());
    photonApp->plugins()->registerNode(SetFixtureZoom::info());
    photonApp->plugins()->registerNode(SetFixtureDimmer::info());
    photonApp->plugins()->registerNode(SetFixtureSlot::info());
    photonApp->plugins()->registerNode(SetFixtureStrobe::info());
    photonApp->plugins()->registerNode(SetFixtureSlotRotation::info());
    photonApp->plugins()->registerNode(SetFixturePrismRotation::info());
    photonApp->plugins()->registerNode(CanvasReader::info());
    //photonApp->plugins()->registerNode(CanvasWriter::info());
    photonApp->plugins()->registerNode(DrawRectangle::info());
    photonApp->plugins()->registerNode(DrawEllipse::info());
    photonApp->plugins()->registerNode(RenderPath::info());
    photonApp->plugins()->registerNode(RenderStroke::info());
    photonApp->plugins()->registerNode(LookAtTarget::info());
    photonApp->plugins()->registerNode(LookInDirection::info());
    photonApp->plugins()->registerNode(MirrorPanTilt::info());
    photonApp->plugins()->registerNode(ScatterBeams::info());
    photonApp->plugins()->registerNode(CircleNode::info());
    photonApp->plugins()->registerNode(NoiseNode::info());
    photonApp->plugins()->registerNode(ColorFromHSV::info());
    photonApp->plugins()->registerNode(RandomColorNode::info());
    photonApp->plugins()->registerNode(MaskNode::info());
    photonApp->plugins()->registerNode(SparkleNode::info());
    photonApp->plugins()->registerNode(TransformTextureNode::info());    
    photonApp->plugins()->registerNode(FixtureListRandomSubsetNode::info());
    photonApp->plugins()->registerNode(FixtureListIntervalSubsetNode::info());
    photonApp->plugins()->registerNode(FixtureListMergeNode::info());
    photonApp->plugins()->registerNode(ColorPaletteNode::info());
    photonApp->plugins()->registerNode(ColorFromColorPalette::info());
    photonApp->plugins()->registerNode(FadeColorPaletteNode::info());
    photonApp->plugins()->registerNode(ColorTheoryPaletteNode::info());
    photonApp->plugins()->registerNode(Timer::info());
    photonApp->plugins()->registerNode(StopwatchNode::info());
    photonApp->plugins()->registerNode(PulseNode::info());
    photonApp->plugins()->registerNode(OscillatorNode::info());
    photonApp->plugins()->registerNode(TimeNode::info());
    photonApp->plugins()->registerNode(BeatReducerNode::info());
    photonApp->plugins()->registerNode(RemapValueNode::info());
    photonApp->plugins()->registerNode(IfNode::info());
    photonApp->plugins()->registerNode(RandomNumberNode::info());
    photonApp->plugins()->registerNode(LinearFalloffNode::info());
    photonApp->plugins()->registerNode(RandomFalloffNode::info());
    photonApp->plugins()->registerNode(SpatialFalloffNode::info());
    photonApp->plugins()->registerNode(SetPixelColor::info());
    photonApp->plugins()->registerNode(PixelsFromFixtureList::info());
    photonApp->plugins()->registerNode(DJConnectorNode::info());

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
