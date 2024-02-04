#ifndef PHOTON_BLURCLIPEFFECT_H
#define PHOTON_BLURCLIPEFFECT_H

#include "sequence/canvasclipeffect.h"

namespace photon {

class BlurClipEffect : public CanvasClipEffect
{
public:
    BlurClipEffect();

    void init() override;
    void evaluate(CanvasClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();


private:
    OpenGLShader *m_shader = nullptr;
    OpenGLShader *m_outputShader = nullptr;
    OpenGLShader *m_centerShader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_BLURCLIPEFFECT_H
