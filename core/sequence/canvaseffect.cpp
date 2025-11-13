#include "canvaseffect.h"
#include "canvasclip.h"
#include "canvaslayergroup.h"
#include "photoncore.h"
#include "surface/canvasaction.h"

namespace photon {

CanvasEffect::CanvasEffect(const QByteArray &t_id) : BaseEffect(t_id)
{

}

CanvasEffect::~CanvasEffect()
{

}

void CanvasEffect::layerChanged(Layer *t_layer)
{
    BaseEffect::layerChanged(t_layer);

    auto group = dynamic_cast<CanvasLayerGroup*>(t_layer->parentGroup());
    if(group)
    {
        group->openGLContext()->makeCurrent(photonApp->surface());
        initializeContext(group->openGLContext(), group->canvas());
    }
}

void CanvasEffect::initializeContext(QOpenGLContext *, Canvas *)
{

}

void CanvasEffect::canvasResized(QOpenGLContext *, Canvas *)
{

}

void CanvasEffect::addedToParent(BaseEffectParent* t_parent)
{
    BaseEffect::addedToParent(t_parent);

    CanvasAction *canvasAction = dynamic_cast<CanvasAction*>(t_parent);
    if(canvasAction)
    {
        canvasAction->openGLContext()->makeCurrent(photonApp->surface());
        initializeContext(canvasAction->openGLContext(), canvasAction->canvas());
    }
}

void CanvasEffect::processChannels(ProcessContext &t_context)
{
    double initialRelativeTime = t_context.globalTime - effectParent()->startTime();

    CanvasEffectEvaluationContext localContext(t_context);
    localContext.relativeTime = initialRelativeTime;
    localContext.openglContext = t_context.openglContext;
    localContext.canvas = t_context.canvas;
    localContext.buffer = t_context.frameBuffer;
    localContext.resources = t_context.resources;


    localContext.relativeTime = initialRelativeTime;
    if(localContext.relativeTime < 0)
        return;

    prepareContext(localContext);
    evaluate(localContext);

}

} // namespace photon
