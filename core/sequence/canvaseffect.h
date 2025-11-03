#ifndef PHOTON_CANVASCLIPEFFECT_H
#define PHOTON_CANVASCLIPEFFECT_H

#include "baseeffect.h"
#include "opengl/openglframebuffer.h"
#include "opengl/openglresources.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasEffectEvaluationContext : public BaseEffectEvaluationContext
{
public:
    CanvasEffectEvaluationContext(ProcessContext &context):BaseEffectEvaluationContext(context){}

    QOpenGLContext *openglContext = nullptr;
    OpenGLFrameBuffer *buffer = nullptr;
    OpenGLResources *resources = nullptr;
    Canvas *canvas = nullptr;
};





class PHOTONCORE_EXPORT CanvasEffect : public BaseEffect
{
public:
    CanvasEffect(const QByteArray &t_id = QByteArray());
    virtual ~CanvasEffect();

    virtual void initializeContext(QOpenGLContext *, Canvas *);
    virtual void canvasResized(QOpenGLContext *, Canvas *);
    virtual void processChannels(ProcessContext &) override;
    virtual void evaluate(CanvasEffectEvaluationContext &) = 0;

protected:

    virtual void layerChanged(Layer*) override;
};

} // namespace photon

#endif // PHOTON_CANVASCLIPEFFECT_H
