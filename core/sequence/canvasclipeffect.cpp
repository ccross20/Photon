#include "canvasclipeffect.h"
#include "canvasclip.h"
#include "canvaslayergroup.h"
#include "photoncore.h"

namespace photon {

CanvasClipEffect::CanvasClipEffect(const QByteArray &t_id) : ClipEffect(t_id)
{

}

CanvasClipEffect::~CanvasClipEffect()
{

}

void CanvasClipEffect::layerChanged(Layer *t_layer)
{
    ClipEffect::layerChanged(t_layer);

    auto group = dynamic_cast<CanvasLayerGroup*>(t_layer->parentGroup());
    if(group)
    {
        group->openGLContext()->makeCurrent(photonApp->surface());
        initializeContext(group->openGLContext(), group->canvas());
    }
}

void CanvasClipEffect::initializeContext(QOpenGLContext *, Canvas *)
{

}

void CanvasClipEffect::canvasResized(QOpenGLContext *, Canvas *)
{

}

void CanvasClipEffect::processChannels(ProcessContext &t_context)
{
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    CanvasClipEffectEvaluationContext localContext(t_context);
    localContext.relativeTime = initialRelativeTime;
    localContext.openglContext = t_context.openglContext;
    localContext.canvas = t_context.canvas;
    localContext.buffer = t_context.frameBuffer;

    localContext.relativeTime = initialRelativeTime;
    if(localContext.relativeTime < 0)
        return;

    prepareContext(localContext);
    evaluate(localContext);

}

} // namespace photon
