#ifndef PHOTON_IMAGECLIPEFFECT_H
#define PHOTON_IMAGECLIPEFFECT_H

#include <QWidget>
#include "sequence/canvaseffect.h"

namespace photon {

class ImageClipEffect : public CanvasEffect
{
public:
    ImageClipEffect();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    OpenGLShader *m_shader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
    QString path;
};

} // namespace photon

#endif // PHOTON_IMAGECLIPEFFECT_H
