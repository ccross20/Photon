#ifndef PHOTON_KALEIDOSCOPECLIPEFFECT_H
#define PHOTON_KALEIDOSCOPECLIPEFFECT_H

#include "sequence/canvasclipeffect.h"

namespace photon {

class KaleidoscopeClipEffect : public CanvasClipEffect
{
public:
    KaleidoscopeClipEffect();

    void init() override;
    void evaluate(CanvasClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    OpenGLShader *m_shader = nullptr;
    OpenGLShader *m_basicShader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_KALEIDOSCOPECLIPEFFECT_H
