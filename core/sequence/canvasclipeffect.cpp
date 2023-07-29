#include "canvasclipeffect.h"
#include "canvasclip.h"

namespace photon {

CanvasClipEffect::CanvasClipEffect(const QByteArray &t_id) : ClipEffect(t_id)
{

}

CanvasClipEffect::~CanvasClipEffect()
{

}

void CanvasClipEffect::processChannels(ProcessContext &t_context) const
{
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    CanvasClipEffectEvaluationContext localContext(t_context);
    localContext.relativeTime = initialRelativeTime;

    for(auto source :  static_cast<CanvasClip*>(clip())->sources())
    {
        if(!source)
            continue;

        localContext.relativeTime = initialRelativeTime;
        if(localContext.relativeTime < 0)
            continue;

        localContext.source = source;

        prepareContext(localContext);

        evaluate(localContext);
    }
}

} // namespace photon
