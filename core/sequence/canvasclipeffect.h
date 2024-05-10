#ifndef PHOTON_CANVASCLIPEFFECT_H
#define PHOTON_CANVASCLIPEFFECT_H

#include "clipeffect.h"
#include "opengl/openglframebuffer.h"
#include "opengl/openglresources.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasClipEffectEvaluationContext : public ClipEffectEvaluationContext
{
public:
    CanvasClipEffectEvaluationContext(ProcessContext &context):ClipEffectEvaluationContext(context){}

    QOpenGLContext *openglContext = nullptr;
    OpenGLFrameBuffer *buffer = nullptr;
    OpenGLResources *resources = nullptr;
    Canvas *canvas = nullptr;
};





class PHOTONCORE_EXPORT CanvasClipEffect : public ClipEffect
{
public:
    CanvasClipEffect(const QByteArray &t_id = QByteArray());
    virtual ~CanvasClipEffect();

    virtual void initializeContext(QOpenGLContext *, Canvas *);
    virtual void canvasResized(QOpenGLContext *, Canvas *);
    virtual void processChannels(ProcessContext &) override;
    virtual void evaluate(CanvasClipEffectEvaluationContext &) = 0;

protected:

    virtual void layerChanged(Layer*) override;
};

} // namespace photon

#endif // PHOTON_CANVASCLIPEFFECT_H
