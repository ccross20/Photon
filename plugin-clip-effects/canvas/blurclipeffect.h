#ifndef PHOTON_BLURCLIPEFFECT_H
#define PHOTON_BLURCLIPEFFECT_H

#include "sequence/canvaseffect.h"

namespace photon {

class BlurClipEffect : public CanvasEffect
{
public:
    BlurClipEffect();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();


private:
    OpenGLShader *m_shader = nullptr;
    OpenGLShader *m_outputShader = nullptr;
    OpenGLShader *m_centerShader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLPingPongTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_BLURCLIPEFFECT_H
