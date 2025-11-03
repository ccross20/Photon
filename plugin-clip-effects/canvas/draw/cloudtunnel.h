#ifndef PHOTON_CLOUDTUNNEL_H
#define PHOTON_CLOUDTUNNEL_H

#include <QWidget>
#include "sequence/canvaseffect.h"

namespace photon {

class CloudTunnel : public CanvasEffect
{
public:
    CloudTunnel();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    OpenGLShader *m_shader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_CLOUDTUNNEL_H
