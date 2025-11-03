#ifndef PHOTON_KALEIDOSCOPECLIPEFFECT_H
#define PHOTON_KALEIDOSCOPECLIPEFFECT_H

#include "sequence/canvaseffect.h"

namespace photon {

class KaleidoscopeClipEffect : public CanvasEffect
{
public:
    KaleidoscopeClipEffect();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    OpenGLShader *m_shader = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_KALEIDOSCOPECLIPEFFECT_H
