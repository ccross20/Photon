#ifndef PHOTON_CANVASCLIPEFFECT_H
#define PHOTON_CANVASCLIPEFFECT_H

#include "clipeffect.h"
#include "opengl/openglframebuffer.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasClipEffectEvaluationContext : public ClipEffectEvaluationContext
{
public:
    CanvasClipEffectEvaluationContext(ProcessContext &context):ClipEffectEvaluationContext(context),canvasImage(context.canvasImage),
        previousCanvasImage(context.previousCanvasImage){}

    QOpenGLContext *openglContext = nullptr;
    QImage *canvasImage = nullptr;
    QImage *previousCanvasImage = nullptr;
    OpenGLFrameBuffer *buffer = nullptr;
    Canvas *canvas = nullptr;
    PixelSource *source = nullptr;
};





class PHOTONCORE_EXPORT CanvasClipEffect : public ClipEffect
{
public:
    CanvasClipEffect(const QByteArray &t_id = QByteArray());
    virtual ~CanvasClipEffect();

    virtual void processChannels(ProcessContext &) override;
    virtual void evaluate(CanvasClipEffectEvaluationContext &) = 0;
};

} // namespace photon

#endif // PHOTON_CANVASCLIPEFFECT_H
