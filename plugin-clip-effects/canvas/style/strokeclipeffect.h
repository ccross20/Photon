#ifndef PHOTON_STROKECLIPEFFECT_H
#define PHOTON_STROKECLIPEFFECT_H

#include "sequence/canvaseffect.h"

namespace photon {

class StrokeClipEffect : public CanvasEffect
{
public:
    StrokeClipEffect();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    OpenGLShader *m_shader = nullptr;
    OpenGLShader *m_circleShader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_STROKECLIPEFFECT_H
