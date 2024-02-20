#ifndef PHOTON_DRAWELLIPSECLIPEFFECT_H
#define PHOTON_DRAWELLIPSECLIPEFFECT_H

#include "sequence/canvasclipeffect.h"

namespace photon {

class DrawEllipseClipEffect : public CanvasClipEffect
{
public:
    DrawEllipseClipEffect();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    OpenGLShader *m_shader = nullptr;
    OpenGLShader *m_circleShader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
    int m_viewportLoc;
    int m_cameraLoc;
    int m_circleViewportLoc;
    int m_circleCameraLoc;
};

} // namespace photon

#endif // PHOTON_DRAWELLIPSECLIPEFFECT_H
