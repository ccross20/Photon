#ifndef PHOTON_CANVASCLIPEFFECT_H
#define PHOTON_CANVASCLIPEFFECT_H

#include "clipeffect.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasClipEffectEvaluationContext : public ClipEffectEvaluationContext
{
public:
    CanvasClipEffectEvaluationContext(ProcessContext &context):ClipEffectEvaluationContext(context),canvasImage(context.canvasImage),
        previousCanvasImage(context.previousCanvasImage){}

    QImage *canvasImage = nullptr;
    QImage *previousCanvasImage = nullptr;
    PixelSource *source = nullptr;
};





class PHOTONCORE_EXPORT CanvasClipEffect : public ClipEffect
{
public:
    CanvasClipEffect(const QByteArray &t_id = QByteArray());
    virtual ~CanvasClipEffect();

    virtual void processChannels(ProcessContext &) const override;
    virtual void evaluate(CanvasClipEffectEvaluationContext &) const = 0;
};

} // namespace photon

#endif // PHOTON_CANVASCLIPEFFECT_H
